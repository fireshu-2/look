#pragma once

#include <vector>
#include <array>
#include <string>
#include <map>
#include <deque>
#include <tuple>

// Parameters
#define MIN_BOX_AREA 800
#define MIN_PERSON_WIDTH 16
#define MIN_PERSON_HEIGHT 38
#define MIN_PERSON_ASPECT 1.05f
#define MAX_PERSON_ASPECT 5.20f
#define IOU_MATCH_THRES 0.20f
#define NMS_IOU_THRES 0.60f
#define MAX_LOST 3
#define MIN_HITS_FOR_EVENT 2
#define MIN_HITS_TO_DRAW 2
#define TRAIL_LEN 20
#define DWELL_SECONDS 3.0f
#define ALARM_COOLDOWN 2.0f
#define ALARM_CONFIRM_FRAMES 3

struct EventRecord {
    std::string timestamp;
    int track_id;
    std::string event_type;
    std::string detail;
};

class EventLogger {
public:
    EventLogger(const std::string& csv_path);
    void write(const EventRecord& record);
private:
    std::string csv_path_;
    void _ensure_header();
};

class AlarmManager {
public:
    AlarmManager(float cooldown = 2.0f);
    bool can_fire(int track_id, const std::string& event_type, double now);
private:
    float cooldown_;
    std::map<std::pair<int, std::string>, double> last_fire_;
};

struct TrackData {
    std::array<float, 4> box;
    float conf;
    int lost;
    int hits;
};

class SimpleZoneTrackManager {
public:
    SimpleZoneTrackManager(int max_lost = 3, float iou_thres = 0.20f);
    std::vector<int> update(const std::vector<std::pair<std::array<float, 4>, float>>& detections);
    std::vector<std::tuple<int, std::array<float, 4>, float, int, int>> get_active();
    bool empty();
private:
    int max_lost_;
    float iou_thres_;
    int next_id_;
    std::map<int, TrackData> tracks_;
    float compute_iou(const std::array<float, 4>& box_a, const std::array<float, 4>& box_b);
};

struct TrackState {
    bool crossed;
    std::string direction;
    bool in_zone;
    bool entered_zone;
    bool dwell_alarm;
    float dwell_time;
    bool is_human_motion;
};

class IntrusionAnalyzer {
public:
    IntrusionAnalyzer(const std::array<float, 2>& line_p1, const std::array<float, 2>& line_p2, const std::vector<std::array<int, 2>>& zone_polygon, float dwell_seconds = 3.0f);

    std::pair<int, int> bbox_anchor_xyxy(const std::array<float, 4>& xyxy);
    float side_of_line(const std::array<float, 2>& p, const std::array<float, 2>& a, const std::array<float, 2>& b);
    bool is_human_like_motion(int track_id, const std::pair<int, int>& current_center);
    bool confirm_alarm_condition(int track_id, const std::string& event_type, bool condition);
    bool confirm_line_cross(int track_id, bool crossed, const std::string& direction, bool still_valid);
    TrackState update_track(int track_id, const std::pair<int, int>& center, double now, bool box_in_zone_now);
    void clear_track(int track_id);

    int line_cross_count = 0;
    int zone_intrusion_count = 0;
    int dwell_alarm_count = 0;
    int line_alarm_count = 0;
    int zone_alarm_count = 0;
    std::map<int, std::deque<std::pair<int, int>>> trails;
    std::map<int, std::string> pending_line_direction;

private:
    std::array<float, 2> line_p1_;
    std::array<float, 2> line_p2_;
    std::vector<std::array<int, 2>> zone_polygon_;
    float dwell_seconds_;

    std::map<int, std::pair<int, int>> prev_centers_;
    std::map<int, double> in_zone_since_;
    std::map<int, bool> zone_state_;
    std::map<int, int> zone_in_frames_;
    std::map<int, int> zone_out_frames_;
    std::map<int, int> non_human_motion_frames_;
    std::map<std::pair<int, std::string>, int> alarm_confirm_frames_;
};

bool bbox_in_zone(const std::array<float, 4>& box, const std::vector<std::array<int, 2>>& polygon);
bool valid_person_box(const std::array<float, 4>& box);
