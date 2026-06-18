#include <obs-module.h>
#include <obs-frontend-api.h>
#include <graphics/graphics.h>
#include <graphics/vec4.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-guide-overlay", "en-US")

#define PROP_PREVIEW_ONLY  "preview_only"
#define PROP_SHOW_THIRDS   "show_thirds"
#define PROP_SHOW_CENTER   "show_center"
#define PROP_SHOW_ACTION   "show_action_safe"
#define PROP_SHOW_TITLE    "show_title_safe"
#define PROP_COLOR         "line_color"
#define PROP_OPACITY       "opacity"
#define PROP_THICKNESS     "thickness"

/* Estado global — atualizado pelo callback de evento */
static volatile bool g_output_active = false;

typedef struct {
	bool     preview_only;
	bool     show_thirds;
	bool     show_center;
	bool     show_action_safe;
	bool     show_title_safe;
	uint32_t color;
	int      opacity;
	int      thickness;
	uint32_t cx;
	uint32_t cy;
} guide_data_t;

static void get_vec4(guide_data_t *d, struct vec4 *out)
{
	uint32_t c = d->color;
	out->x = ((c >> 16) & 0xFF) / 255.0f;
	out->y = ((c >>  8) & 0xFF) / 255.0f;
	out->z = ( c        & 0xFF) / 255.0f;
	out->w = (float)d->opacity / 100.0f;
}

static void draw_hline(float x0, float x1, float y, float half)
{
	gs_render_start(false);
	gs_vertex2f(x0, y-half); gs_vertex2f(x1, y-half); gs_vertex2f(x1, y+half);
	gs_vertex2f(x0, y-half); gs_vertex2f(x1, y+half); gs_vertex2f(x0, y+half);
	gs_render_stop(GS_TRIS);
}

static void draw_vline(float x, float y0, float y1, float half)
{
	gs_render_start(false);
	gs_vertex2f(x-half, y0); gs_vertex2f(x+half, y0); gs_vertex2f(x+half, y1);
	gs_vertex2f(x-half, y0); gs_vertex2f(x+half, y1); gs_vertex2f(x-half, y1);
	gs_render_stop(GS_TRIS);
}

/* Callback chamado pelo OBS quando eventos acontecem */
static void frontend_event_cb(enum obs_frontend_event event, void *private_data)
{
	UNUSED_PARAMETER(private_data);
	switch (event) {
	case OBS_FRONTEND_EVENT_RECORDING_STARTED:
	case OBS_FRONTEND_EVENT_STREAMING_STARTED:
	case OBS_FRONTEND_EVENT_VIRTUALCAM_STARTED:
		g_output_active = true;
		break;
	case OBS_FRONTEND_EVENT_RECORDING_STOPPED:
	case OBS_FRONTEND_EVENT_STREAMING_STOPPED:
	case OBS_FRONTEND_EVENT_VIRTUALCAM_STOPPED:
		g_output_active = false;
		break;
	default:
		break;
	}
}

static const char *guide_get_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return obs_module_text("PluginName");
}

static void *guide_create(obs_data_t *settings, obs_source_t *source)
{
	UNUSED_PARAMETER(source);
	guide_data_t *d = bzalloc(sizeof(guide_data_t));
	struct obs_video_info ovi;
	if (obs_get_video_info(&ovi)) { d->cx = ovi.base_width; d->cy = ovi.base_height; }
	else { d->cx = 1920; d->cy = 1080; }
	obs_source_update(source, settings);
	return d;
}

static void guide_destroy(void *data) { bfree(data); }

static void guide_update(void *data, obs_data_t *s)
{
	guide_data_t *d = data;
	d->preview_only     = obs_data_get_bool(s, PROP_PREVIEW_ONLY);
	d->show_thirds      = obs_data_get_bool(s, PROP_SHOW_THIRDS);
	d->show_center      = obs_data_get_bool(s, PROP_SHOW_CENTER);
	d->show_action_safe = obs_data_get_bool(s, PROP_SHOW_ACTION);
	d->show_title_safe  = obs_data_get_bool(s, PROP_SHOW_TITLE);
	d->color            = (uint32_t)obs_data_get_int(s, PROP_COLOR);
	d->opacity          = (int)obs_data_get_int(s, PROP_OPACITY);
	d->thickness        = (int)obs_data_get_int(s, PROP_THICKNESS);
}

static uint32_t guide_get_width(void *data)  { return ((guide_data_t *)data)->cx; }
static uint32_t guide_get_height(void *data) { return ((guide_data_t *)data)->cy; }

static void guide_render(void *data, gs_effect_t *effect)
{
	UNUSED_PARAMETER(effect);
	guide_data_t *d = data;

	if (d->preview_only && g_output_active)
		return;

	float W    = (float)d->cx;
	float H    = (float)d->cy;
	float half = (d->thickness > 0 ? d->thickness : 2) * 0.5f;

	struct vec4 col;
	get_vec4(d, &col);

	gs_effect_t *solid = obs_get_base_effect(OBS_EFFECT_SOLID);
	gs_effect_set_vec4(gs_effect_get_param_by_name(solid, "color"), &col);

	while (gs_effect_loop(solid, "Solid")) {
		if (d->show_thirds) {
			for (int i = 1; i < 3; i++) {
				draw_vline(W * i / 3.0f, 0, H, half);
				draw_hline(0, W, H * i / 3.0f, half);
			}
		}
		if (d->show_center) {
			float cx = W/2, cy = H/2, arm = W*0.05f;
			draw_hline(cx-arm, cx+arm, cy, half);
			draw_vline(cx, cy-arm, cy+arm, half);
		}
		if (d->show_action_safe) {
			float mx = W*0.05f, my = H*0.05f;
			draw_hline(mx, W-mx, my, half); draw_hline(mx, W-mx, H-my, half);
			draw_vline(mx, my, H-my, half); draw_vline(W-mx, my, H-my, half);
		}
		if (d->show_title_safe) {
			float mx = W*0.10f, my = H*0.10f;
			draw_hline(mx, W-mx, my, half); draw_hline(mx, W-mx, H-my, half);
			draw_vline(mx, my, H-my, half); draw_vline(W-mx, my, H-my, half);
		}
	}
}

static obs_properties_t *guide_get_properties(void *unused)
{
	UNUSED_PARAMETER(unused);
	obs_properties_t *props = obs_properties_create();
	obs_properties_add_bool(props, PROP_PREVIEW_ONLY, obs_module_text("PreviewOnly"));
	obs_properties_add_bool(props, PROP_SHOW_THIRDS,  obs_module_text("ShowThirds"));
	obs_properties_add_bool(props, PROP_SHOW_CENTER,  obs_module_text("ShowCenter"));
	obs_properties_add_bool(props, PROP_SHOW_ACTION,  obs_module_text("ShowActionSafe"));
	obs_properties_add_bool(props, PROP_SHOW_TITLE,   obs_module_text("ShowTitleSafe"));
	obs_properties_add_color(props, PROP_COLOR,       obs_module_text("LineColor"));
	obs_property_t *op = obs_properties_add_int_slider(props, PROP_OPACITY, obs_module_text("Opacity"), 0, 100, 1);
	obs_property_int_set_suffix(op, " %");
	obs_properties_add_int_slider(props, PROP_THICKNESS, obs_module_text("Thickness"), 1, 20, 1);
	return props;
}

static void guide_get_defaults(obs_data_t *s)
{
	obs_data_set_default_bool(s, PROP_PREVIEW_ONLY, true);
	obs_data_set_default_bool(s, PROP_SHOW_THIRDS,  true);
	obs_data_set_default_bool(s, PROP_SHOW_CENTER,  true);
	obs_data_set_default_bool(s, PROP_SHOW_ACTION,  false);
	obs_data_set_default_bool(s, PROP_SHOW_TITLE,   false);
	obs_data_set_default_int (s, PROP_COLOR,    0xFFFFFF);
	obs_data_set_default_int (s, PROP_OPACITY,  70);
	obs_data_set_default_int (s, PROP_THICKNESS, 2);
}

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
	obs_frontend_add_event_callback(frontend_event_cb, NULL);
	blog(LOG_INFO, "obs-guide-overlay carregado");
	return true;
}

void obs_module_unload(void)
{
	obs_frontend_remove_event_callback(frontend_event_cb, NULL);
	blog(LOG_INFO, "obs-guide-overlay descarregado");
}
