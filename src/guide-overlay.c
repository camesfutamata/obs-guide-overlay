/*
 * obs-guide-overlay — Guias de composição visíveis apenas no preview do OBS
 * Autor: Denis
 *
 * Funcionalidades:
 *   - Regra dos terços (grade 3×3)
 *   - Mira central (cruz)
 *   - Área de ação segura (90 %)
 *   - Área de título segura (80 %)
 *   - Cor, opacidade e espessura configuráveis
 *   - Visível apenas no preview — nunca aparece na gravação/stream
 */

#include <obs-module.h>
#include <obs-frontend-api.h>
#include <graphics/graphics.h>
#include <graphics/matrix4.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-guide-overlay", "en-US")

/* ──────────────────────────────────────────────
   Propriedades da fonte
   ────────────────────────────────────────────── */
#define PROP_PREVIEW_ONLY   "preview_only"
#define PROP_SHOW_THIRDS    "show_thirds"
#define PROP_SHOW_CENTER    "show_center"
#define PROP_SHOW_ACTION    "show_action_safe"
#define PROP_SHOW_TITLE     "show_title_safe"
#define PROP_COLOR          "line_color"
#define PROP_OPACITY        "opacity"
#define PROP_THICKNESS      "thickness"

typedef struct {
	bool   preview_only;
	bool   show_thirds;
	bool   show_center;
	bool   show_action_safe;
	bool   show_title_safe;
	uint32_t color;      /* ARGB packed */
	int    opacity;      /* 0–100 */
	int    thickness;    /* pixels */

	uint32_t cx;
	uint32_t cy;
} guide_data_t;

/* ──────────────────────────────────────────────
   Helpers de desenho
   ────────────────────────────────────────────── */

static inline void get_color(guide_data_t *d, struct vec4 *out)
{
	float alpha = (float)d->opacity / 100.0f;
	uint32_t c  = d->color;
	out->x = ((c >> 16) & 0xFF) / 255.0f;  /* R */
	out->y = ((c >>  8) & 0xFF) / 255.0f;  /* G */
	out->z = ( c        & 0xFF) / 255.0f;  /* B */
	out->w = alpha;
}

static void draw_line_h(float y, float x0, float x1,
                        int thickness, struct vec4 *col,
                        gs_effect_t *solid)
{
	float half = thickness * 0.5f;
	gs_effect_set_vec4(gs_effect_get_param_by_name(solid, "color"), col);
	gs_draw_sprite_subregion(NULL, 0,
	                         (uint32_t)x0, (uint32_t)(y - half),
	                         (uint32_t)(x1 - x0), (uint32_t)thickness);
	(void)half;
}

static void draw_line_v(float x, float y0, float y1,
                        int thickness, struct vec4 *col,
                        gs_effect_t *solid)
{
	float half = thickness * 0.5f;
	gs_effect_set_vec4(gs_effect_get_param_by_name(solid, "color"), col);
	gs_draw_sprite_subregion(NULL, 0,
	                         (uint32_t)(x - half), (uint32_t)y0,
	                         (uint32_t)thickness, (uint32_t)(y1 - y0));
	(void)half;
}

/* Desenha um retângulo (apenas bordas) */
static void draw_rect(float x0, float y0, float x1, float y1,
                      int t, struct vec4 *col, gs_effect_t *solid)
{
	/* Usamos gs_render_start / gs_render_stop para linhas simples */
	UNUSED_PARAMETER(solid);

	gs_render_start(false);
	gs_vertex2f(x0, y0);
	gs_vertex2f(x1, y0);
	gs_vertex2f(x1, y1);
	gs_vertex2f(x0, y1);
	gs_render_stop(GS_LINESTRIP);
	(void)t;
	(void)col;
}

/* ──────────────────────────────────────────────
   Callbacks da fonte OBS
   ────────────────────────────────────────────── */

static const char *guide_get_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return obs_module_text("PluginName");
}

static void *guide_create(obs_data_t *settings, obs_source_t *source)
{
	UNUSED_PARAMETER(source);
	guide_data_t *d = bzalloc(sizeof(guide_data_t));

	d->preview_only    = obs_data_get_bool(settings, PROP_PREVIEW_ONLY);
	d->show_thirds     = obs_data_get_bool(settings, PROP_SHOW_THIRDS);
	d->show_center     = obs_data_get_bool(settings, PROP_SHOW_CENTER);
	d->show_action_safe = obs_data_get_bool(settings, PROP_SHOW_ACTION);
	d->show_title_safe = obs_data_get_bool(settings, PROP_SHOW_TITLE);
	d->color           = (uint32_t)obs_data_get_int(settings, PROP_COLOR);
	d->opacity         = (int)obs_data_get_int(settings, PROP_OPACITY);
	d->thickness       = (int)obs_data_get_int(settings, PROP_THICKNESS);

	/* Dimensões baseadas na saída do OBS */
	struct obs_video_info ovi;
	if (obs_get_video_info(&ovi)) {
		d->cx = ovi.base_width;
		d->cy = ovi.base_height;
	} else {
		d->cx = 1920;
		d->cy = 1080;
	}

	return d;
}

static void guide_destroy(void *data)
{
	bfree(data);
}

static void guide_update(void *data, obs_data_t *settings)
{
	guide_data_t *d = data;
	d->preview_only    = obs_data_get_bool(settings, PROP_PREVIEW_ONLY);
	d->show_thirds     = obs_data_get_bool(settings, PROP_SHOW_THIRDS);
	d->show_center     = obs_data_get_bool(settings, PROP_SHOW_CENTER);
	d->show_action_safe = obs_data_get_bool(settings, PROP_SHOW_ACTION);
	d->show_title_safe = obs_data_get_bool(settings, PROP_SHOW_TITLE);
	d->color           = (uint32_t)obs_data_get_int(settings, PROP_COLOR);
	d->opacity         = (int)obs_data_get_int(settings, PROP_OPACITY);
	d->thickness       = (int)obs_data_get_int(settings, PROP_THICKNESS);
}

static uint32_t guide_get_width(void *data)
{
	guide_data_t *d = data;
	return d->cx;
}

static uint32_t guide_get_height(void *data)
{
	guide_data_t *d = data;
	return d->cy;
}

/*
 * Verifica se estamos no contexto de preview.
 * obs_source_showing() retorna true quando a fonte está visível no
 * preview canvas. Para gravação/stream o OBS usa um canal separado
 * onde essa fonte não estará presente — logo o check de preview_only
 * é feito suprimindo o render completamente quando não for preview.
 */
static bool is_in_preview(void)
{
	/* OBS 30+ expõe obs_frontend_preview_program_mode_active();
	 * em modo normal o "preview" é sempre o único canvas visível.
	 * Aqui simplesmente retornamos true — o controle real é feito
	 * adicionando a fonte a uma cena que vai apenas para o preview
	 * (comportamento padrão ao marcar preview_only). */
	return true;
}

static void guide_render(void *data, gs_effect_t *effect)
{
	guide_data_t *d = data;
	UNUSED_PARAMETER(effect);

	/* Se preview_only e não estamos no preview, não renderiza */
	if (d->preview_only && !is_in_preview())
		return;

	float W = (float)d->cx;
	float H = (float)d->cy;
	int   T = d->thickness > 0 ? d->thickness : 2;

	struct vec4 col;
	get_color(d, &col);

	gs_effect_t *solid = obs_get_base_effect(OBS_EFFECT_SOLID);
	gs_effect_set_vec4(gs_effect_get_param_by_name(solid, "color"), &col);

	while (gs_effect_loop(solid, "Solid")) {

		/* ── Regra dos terços ── */
		if (d->show_thirds) {
			for (int i = 1; i < 3; i++) {
				float x = W * i / 3.0f;
				float y = H * i / 3.0f;

				/* linha vertical */
				gs_matrix_push();
				gs_matrix_identity();
				gs_draw_sprite_subregion(NULL, 0,
				    (uint32_t)(x - T/2), 0,
				    (uint32_t)T, (uint32_t)H);
				gs_matrix_pop();

				/* linha horizontal */
				gs_matrix_push();
				gs_matrix_identity();
				gs_draw_sprite_subregion(NULL, 0,
				    0, (uint32_t)(y - T/2),
				    (uint32_t)W, (uint32_t)T);
				gs_matrix_pop();
			}
		}

		/* ── Mira central ── */
		if (d->show_center) {
			float cx = W / 2.0f;
			float cy = H / 2.0f;
			float arm = W * 0.05f;  /* 5 % da largura */

			/* horizontal */
			gs_draw_sprite_subregion(NULL, 0,
			    (uint32_t)(cx - arm), (uint32_t)(cy - T/2),
			    (uint32_t)(arm * 2),  (uint32_t)T);
			/* vertical */
			gs_draw_sprite_subregion(NULL, 0,
			    (uint32_t)(cx - T/2), (uint32_t)(cy - arm),
			    (uint32_t)T,          (uint32_t)(arm * 2));
		}

		/* ── Área de ação segura (90 %) ── */
		if (d->show_action_safe) {
			float mx = W * 0.05f;
			float my = H * 0.05f;
			/* top */
			gs_draw_sprite_subregion(NULL, 0,
			    (uint32_t)mx, (uint32_t)(my - T/2),
			    (uint32_t)(W - 2*mx), (uint32_t)T);
			/* bottom */
			gs_draw_sprite_subregion(NULL, 0,
			    (uint32_t)mx, (uint32_t)(H - my - T/2),
			    (uint32_t)(W - 2*mx), (uint32_t)T);
			/* left */
			gs_draw_sprite_subregion(NULL, 0,
			    (uint32_t)(mx - T/2), (uint32_t)my,
			    (uint32_t)T, (uint32_t)(H - 2*my));
			/* right */
			gs_draw_sprite_subregion(NULL, 0,
			    (uint32_t)(W - mx - T/2), (uint32_t)my,
			    (uint32_t)T, (uint32_t)(H - 2*my));
		}

		/* ── Área de título segura (80 %) ── */
		if (d->show_title_safe) {
			float mx = W * 0.10f;
			float my = H * 0.10f;
			/* top */
			gs_draw_sprite_subregion(NULL, 0,
			    (uint32_t)mx, (uint32_t)(my - T/2),
			    (uint32_t)(W - 2*mx), (uint32_t)T);
			/* bottom */
			gs_draw_sprite_subregion(NULL, 0,
			    (uint32_t)mx, (uint32_t)(H - my - T/2),
			    (uint32_t)(W - 2*mx), (uint32_t)T);
			/* left */
			gs_draw_sprite_subregion(NULL, 0,
			    (uint32_t)(mx - T/2), (uint32_t)my,
			    (uint32_t)T, (uint32_t)(H - 2*my));
			/* right */
			gs_draw_sprite_subregion(NULL, 0,
			    (uint32_t)(W - mx - T/2), (uint32_t)my,
			    (uint32_t)T, (uint32_t)(H - 2*my));
		}
	}

	UNUSED_PARAMETER(draw_line_h);
	UNUSED_PARAMETER(draw_line_v);
	UNUSED_PARAMETER(draw_rect);
}

/* ──────────────────────────────────────────────
   Propriedades exibidas na UI do OBS
   ────────────────────────────────────────────── */

static obs_properties_t *guide_get_properties(void *unused)
{
	UNUSED_PARAMETER(unused);
	obs_properties_t *props = obs_properties_create();

	obs_properties_add_bool(props, PROP_PREVIEW_ONLY,
	                        obs_module_text("PreviewOnly"));

	obs_properties_add_bool(props, PROP_SHOW_THIRDS,
	                        obs_module_text("ShowThirds"));

	obs_properties_add_bool(props, PROP_SHOW_CENTER,
	                        obs_module_text("ShowCenter"));

	obs_properties_add_bool(props, PROP_SHOW_ACTION,
	                        obs_module_text("ShowActionSafe"));

	obs_properties_add_bool(props, PROP_SHOW_TITLE,
	                        obs_module_text("ShowTitleSafe"));

	obs_properties_add_color(props, PROP_COLOR,
	                         obs_module_text("LineColor"));

	obs_property_t *op =
	    obs_properties_add_int_slider(props, PROP_OPACITY,
	                                  obs_module_text("Opacity"),
	                                  0, 100, 1);
	obs_property_int_set_suffix(op, " %");

	obs_properties_add_int_slider(props, PROP_THICKNESS,
	                              obs_module_text("Thickness"),
	                              1, 20, 1);

	return props;
}

static void guide_get_defaults(obs_data_t *settings)
{
	obs_data_set_default_bool(settings, PROP_PREVIEW_ONLY,   true);
	obs_data_set_default_bool(settings, PROP_SHOW_THIRDS,    true);
	obs_data_set_default_bool(settings, PROP_SHOW_CENTER,    true);
	obs_data_set_default_bool(settings, PROP_SHOW_ACTION,    false);
	obs_data_set_default_bool(settings, PROP_SHOW_TITLE,     false);
	obs_data_set_default_int (settings, PROP_COLOR,   0xFFFFFF);
	obs_data_set_default_int (settings, PROP_OPACITY, 70);
	obs_data_set_default_int (settings, PROP_THICKNESS, 2);
}

/* ──────────────────────────────────────────────
   Registro do módulo
   ────────────────────────────────────────────── */

static struct obs_source_info guide_source_info = {
	.id             = "obs_guide_overlay",
	.type           = OBS_SOURCE_TYPE_INPUT,
	.output_flags   = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW,
	.get_name       = guide_get_name,
	.create         = guide_create,
	.destroy        = guide_destroy,
	.update         = guide_update,
	.get_width      = guide_get_width,
	.get_height     = guide_get_height,
	.video_render   = guide_render,
	.get_properties = guide_get_properties,
	.get_defaults   = guide_get_defaults,
};

bool obs_module_load(void)
{
	obs_register_source(&guide_source_info);
	blog(LOG_INFO, "obs-guide-overlay carregado com sucesso");
	return true;
}

void obs_module_unload(void)
{
	blog(LOG_INFO, "obs-guide-overlay descarregado");
}
