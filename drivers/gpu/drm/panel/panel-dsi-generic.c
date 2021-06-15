// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Markus Bauer <mb@karo-electronics.de>
 * 
 * DRM driver for a generic MIPI DSI panel dummy object.
 *
 */

#include <linux/backlight.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/regulator/consumer.h>

#include <video/mipi_display.h>

#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>
#include <drm/drm_print.h>

struct panel_generic {
	struct device *dev;
	struct drm_panel panel;
};

static inline struct panel_generic *panel_to_panel_generic(struct drm_panel *panel)
{
	return container_of(panel, struct panel_generic, panel);
}

static int panel_generic_disable(struct drm_panel *panel)
{
	struct panel_generic *p = panel_to_panel_generic(panel);
	struct mipi_dsi_device *dsi = to_mipi_dsi_device(p->dev);
	int ret;

	ret = mipi_dsi_dcs_set_display_off(dsi);
	if (ret)
		return ret;

	return 0;
}

static int panel_generic_unprepare(struct drm_panel *panel)
{
	return 0;
}

static int panel_generic_prepare(struct drm_panel *panel)
{
	return 0;
}

static int panel_generic_enable(struct drm_panel *panel)
{
	return 0;
}

static int panel_generic_get_modes(struct drm_panel *panel,
			      struct drm_connector *connector)
{
	return 0;
}

static const struct drm_panel_funcs panel_generic_drm_funcs = {
	.disable   = panel_generic_disable,
	.unprepare = panel_generic_unprepare,
	.prepare   = panel_generic_prepare,
	.enable    = panel_generic_enable,
	.get_modes = panel_generic_get_modes,
};

static int panel_generic_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct panel_generic *p;
	int ret;

	p = devm_kzalloc(dev, sizeof(*p), GFP_KERNEL);
	if (!p)
		return -ENOMEM;

	mipi_dsi_set_drvdata(dsi, p);

	p->dev = dev;

	dsi->lanes = 2;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_BURST;

	drm_panel_init(&p->panel, dev, &panel_generic_drm_funcs,
		       DRM_MODE_CONNECTOR_DSI);

	drm_panel_add(&p->panel);

	ret = mipi_dsi_attach(dsi);
	if (ret < 0) {
		// only error when not retrying
		if (ret != -EPROBE_DEFER)
			dev_err(dev, "mipi_dsi_attach failed.\n");

		drm_panel_remove(&p->panel);
		return ret;
	}

	return 0;
}

static int panel_generic_remove(struct mipi_dsi_device *dsi)
{
	struct panel_generic *p = mipi_dsi_get_drvdata(dsi);

	mipi_dsi_detach(dsi);
	drm_panel_remove(&p->panel);

	return 0;
}

static const struct of_device_id panel_generic_of_match[] = {
	{ .compatible = "dsi,panel-generic" },
	{ }
};
MODULE_DEVICE_TABLE(of, panel_generic_of_match);

static struct mipi_dsi_driver panel_generic_driver = {
	.probe  = panel_generic_probe,
	.remove = panel_generic_remove,
	.driver = {
		.name = "panel-dsi-generic",
		.of_match_table = panel_generic_of_match,
	},
};
module_mipi_dsi_driver(panel_generic_driver);

MODULE_AUTHOR("Markus Bauer <mb@karo-electronics.de>");
MODULE_DESCRIPTION("DRM driver for a generic MIPI DSI panel dummy object.");
MODULE_LICENSE("GPL v2");
