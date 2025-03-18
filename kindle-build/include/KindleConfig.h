#pragma once

// This file contains Kindle-specific configurations and optimizations

#ifdef KINDLE

// E-ink display refresh function
#include <cstdio>
#include <cstdlib>

namespace KindleUtils {

// Refresh the e-ink display
inline void refreshDisplay() {
    // Write to the Kindle's e-ink refresh control file
    FILE* fp = fopen("/proc/eink_fb/update_display", "w");
    if (fp) {
        fprintf(fp, "1");
        fclose(fp);
    }
}

// Set the display to fast refresh mode (for typing)
inline void setFastRefresh() {
    system("echo A > /proc/eink_fb/update_mode");
}

// Set the display to high quality refresh mode (for reading)
inline void setHighQualityRefresh() {
    system("echo U > /proc/eink_fb/update_mode");
}

// Kindle screen dimensions (adjust based on your target Kindle model)
constexpr int SCREEN_WIDTH = 600;
constexpr int SCREEN_HEIGHT = 800;

// Optimize UI for e-ink display
constexpr bool USE_DARK_THEME = false;  // Light theme is better for e-ink
constexpr int DEFAULT_FONT_SIZE = 14;   // Larger font for readability
constexpr int REFRESH_INTERVAL = 500;   // Milliseconds between display refreshes

} // namespace KindleUtils

#endif // KINDLE 