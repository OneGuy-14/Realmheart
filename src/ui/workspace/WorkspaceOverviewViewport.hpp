#pragma once

#include <algorithm>
#include <cmath>

#include "ui/bar/BarGeometry.hpp"

namespace realmheart::ui::workspace {

struct WorkspaceOverviewViewportTransform {
    double scale = 1.0;
    double content_width = 1920.0;
    double content_height = 1080.0;
    // Widget-space y of the stage top. Non-zero when a horizontal bar
    // reserves the top of the output.
    double origin_y = 0.0;
    // Widget-space x of the stage left edge. Non-zero when the stage is
    // narrower than the output, which happens once height is the limiting
    // dimension.
    double origin_x = 0.0;

    [[nodiscard]] double to_reference_x(double x) const noexcept {
        return (x - origin_x) / scale;
    }

    [[nodiscard]] double to_reference_y(double y) const noexcept {
        return (y - origin_y) / scale;
    }

    [[nodiscard]] double to_widget_x(double x) const noexcept {
        return x * scale + origin_x;
    }

    [[nodiscard]] double to_widget_y(double y) const noexcept {
        return y * scale + origin_y;
    }
};

[[nodiscard]] inline WorkspaceOverviewViewportTransform
workspace_overview_viewport_transform(
    int logical_width,
    int logical_height,
    double reference_width = 1920.0,
    double reference_height = 1080.0
) noexcept {
    if (logical_width <= 0 || logical_height <= 0 ||
        !std::isfinite(reference_width) || !std::isfinite(reference_height) ||
        reference_width <= 0.0 || reference_height <= 0.0) {
        return {};
    }

    // The authored overview is a 16:9 stage.  Use a single uniform scale so
    // ultrawide outputs gain horizontal breathing room instead of stretching
    // every realm, card and character.  Keep the stage top-left anchored so
    // its Aether Spine morph coordinates continue to line up with the bar.
    // A horizontal bar reserves the top of the output. Fit the stage into
    // what is left and push it below the bar, or the first rows draw under
    // it while dead space collects at the bottom.
    const double top_inset = static_cast<double>(
        bar::bar_geometry_for_logical_geometry(
            logical_width, logical_height
        ).rail_width
    );
    const double usable_height =
        std::max(static_cast<double>(logical_height) - top_inset, 1.0);

    // Scale on width alone: losing height to the bar would otherwise shrink
    // the stage sideways too, leaving visible slack on a 16:9 output. The
    // overflow falls off the bottom, which is quieter than side gaps.
    const double scale = std::min(
        static_cast<double>(logical_width) / reference_width,
        usable_height / reference_height
    );
    if (!std::isfinite(scale) || scale <= 0.0) return {};

    const double content_height = reference_height * scale;
    const double content_width = reference_width * scale;
    return {
        .scale = scale,
        .content_width = content_width,
        .content_height = content_height,
        // Sit directly under the bar; let all the 16:9-into-3:2 slack
        // collect in one gap at the bottom rather than two thin ones.
        .origin_y = top_inset,
        // Centre horizontally. When height is the limiting dimension the
        // stage is narrower than the output, and left-anchoring pools every
        // spare pixel on the right - visible on a 16:9 external monitor.
        .origin_x = std::max(
            (static_cast<double>(logical_width) - content_width) * 0.5, 0.0
        ),
    };
}

} // namespace realmheart::ui::workspace
