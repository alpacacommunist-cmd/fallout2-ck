#ifndef CK_PUBLIC_MAP_H
#define CK_PUBLIC_MAP_H

namespace ck::map {
	void on_map_enter();
	void on_before_map_load();
	bool map_has_camera_borders(int map_index);
	bool map_is_camera_position_allowed(int tile);
}

#endif
