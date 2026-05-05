#include "intrusion_system.hpp"
#include <fstream>
#include <cmath>
#include <algorithm>
#include <iostream>

// --- EventLogger ---
EventLogger::EventLogger(const std::string& csv_path) : csv_path_(csv_path) {
    _ensure_header();
}

void EventLogger::_ensure_header() {
    std::ifstream f(csv_path_);
    if (!f.good()) {
        std::ofstream out(csv_path_);
        out << "timestamp,track_id,event_type,detail\n";
    }
}

void EventLogger::write(const EventRecord& record) {
    std::ofstream out(csv_path_, std::ios_base::app);
    out << record.timestamp << "," << record.track_id << "," << record.event_type << "," << record.detail << "\n";
}

// --- AlarmManager ---
AlarmManager::AlarmManager(float cooldown) : cooldown_(cooldown) {}

bool AlarmManager::can_fire(int track_id, const std::string& event_type, double now) {
    auto key = std::make_pair(track_id, event_type);
    double last_t = 0.0;
    if (last_fire_.find(key) != last_fire_.end()) {
        last_t = last_fire_[key];
    }
    if (now - last_t >= cooldown_) {
        last_fire_[key] = now;
        return true;
    }
    return false;
}

// --- SimpleZoneTrackManager ---
SimpleZoneTrackManager::SimpleZoneTrackManager(int max_lost, float iou_thres)
    : max_lost_(max_lost), iou_thres_(iou_thres), next_id_(1) {}

float SimpleZoneTrackManager::compute_iou(const std::array<float, 4>& box_a, const std::array<float, 4>& box_b) {
    float x_a = std::max(box_a[0], box_b[0]);
    float y_a = std::max(box_a[1], box_b[1]);
    float x_b = std::min(box_a[2], box_b[2]);
    float y_b = std::min(box_a[3], box_b[3]);

    float inter_w = std::max(0.0f, x_b - x_a);
    float inter_h = std::max(0.0f, y_b - y_a);
    float inter = inter_w * inter_h;

    float area_a = std::max(0.0f, box_a[2] - box_a[0]) * std::max(0.0f, box_a[3] - box_a[1]);
    float area_b = std::max(0.0f, box_b[2] - box_b[0]) * std::max(0.0f, box_b[3] - box_b[1]);

    return inter / (area_a + area_b - inter + 1e-6f);
}

std::vector<int> SimpleZoneTrackManager::update(const std::vector<std::pair<std::array<float, 4>, float>>& detections) {
    std::vector<int> removed;
    std::vector<int> matched_tracks;
    std::vector<int> matched_dets;

    for (size_t i = 0; i < detections.size(); ++i) {
        float best_iou = 0.0f;
        int best_tid = -1;

        for (auto& kv : tracks_) {
            int tid = kv.first;
            if (std::find(matched_tracks.begin(), matched_tracks.end(), tid) != matched_tracks.end()) continue;

            float iou = compute_iou(detections[i].first, kv.second.box);
            if (iou > best_iou) {
                best_iou = iou;
                best_tid = tid;
            }
        }

        if (best_tid != -1 && best_iou >= iou_thres_) {
            tracks_[best_tid].box = detections[i].first;
            tracks_[best_tid].conf = detections[i].second;
            tracks_[best_tid].lost = 0;
            tracks_[best_tid].hits += 1;
            matched_tracks.push_back(best_tid);
            matched_dets.push_back(i);
        }
    }

    for (size_t i = 0; i < detections.size(); ++i) {
        if (std::find(matched_dets.begin(), matched_dets.end(), i) == matched_dets.end()) {
            int tid = next_id_++;
            tracks_[tid] = {detections[i].first, detections[i].second, 0, 1};
        }
    }

    std::vector<int> track_ids;
    for (const auto& kv : tracks_) track_ids.push_back(kv.first);

    for (int tid : track_ids) {
        if (std::find(matched_tracks.begin(), matched_tracks.end(), tid) == matched_tracks.end()) {
            tracks_[tid].lost += 1;
            if (tracks_[tid].lost > max_lost_) {
                tracks_.erase(tid);
                removed.push_back(tid);
            }
        }
    }
    return removed;
}

std::vector<std::tuple<int, std::array<float, 4>, float, int, int>> SimpleZoneTrackManager::get_active() {
    std::vector<std::tuple<int, std::array<float, 4>, float, int, int>> result;
    for (const auto& kv : tracks_) {
        result.push_back(std::make_tuple(kv.first, kv.second.box, kv.second.conf, kv.second.hits, kv.second.lost));
    }
    return result;
}

bool SimpleZoneTrackManager::empty() {
    return tracks_.empty();
}

// --- Point In Polygon (Ray-casting) ---
static bool point_in_polygon(const std::array<int, 2>& pt, const std::vector<std::array<int, 2>>& poly) {
    bool inside = false;
    for (size_t i = 0, j = poly.size() - 1; i < poly.size(); j = i++) {
        if (((poly[i][1] > pt[1]) != (poly[j][1] > pt[1])) &&
            (pt[0] < (poly[j][0] - poly[i][0]) * (pt[1] - poly[i][1]) / (float)(poly[j][1] - poly[i][1] + 1e-6) + poly[i][0])) {
            inside = !inside;
        }
    }
    return inside;
}

// --- Helper Funcs ---
bool bbox_in_zone(const std::array<float, 4>& box, const std::vector<std::array<int, 2>>& polygon) {
    int x1 = box[0], y1 = box[1], x2 = box[2], y2 = box[3];
    int w = std::max(1, x2 - x1);
    int h = std::max(1, y2 - y1);

    std::vector<std::array<int, 2>> points = {
        {(x1 + x2) / 2, y2},
        {int(x1 + 0.30 * w), y2},
        {int(x1 + 0.70 * w), y2},
        {(x1 + x2) / 2, int(y1 + 0.85 * h)},
        {int(x1 + 0.35 * w), int(y1 + 0.85 * h)},
        {int(x1 + 0.65 * w), int(y1 + 0.85 * h)},
        {(x1 + x2) / 2, int(y1 + 0.70 * h)},
        {int(x1 + 0.35 * w), int(y1 + 0.70 * h)},
        {int(x1 + 0.65 * w), int(y1 + 0.70 * h)}
    };

    int inside_count = 0;
    for (const auto& p : points) {
        if (point_in_polygon(p, polygon)) {
            inside_count++;
        }
    }

    // We simplified overlap_ratio check for C++, rely on inside_count >= 2
    return inside_count >= 2;
}

bool valid_person_box(const std::array<float, 4>& box) {
    float w = std::max(1.0f, box[2] - box[0]);
    float h = std::max(1.0f, box[3] - box[1]);
    float area = w * h;
    float aspect = h / w;

    if (area < MIN_BOX_AREA) return false;
    if (w < MIN_PERSON_WIDTH) return false;
    if (h < MIN_PERSON_HEIGHT) return false;
    if (aspect < MIN_PERSON_ASPECT || aspect > MAX_PERSON_ASPECT) return false;

    if (area < 1600 && w < 22 && aspect > 3.8) return false;
    if (w < 14 && aspect > 4.5) return false;

    return true;
}

// --- IntrusionAnalyzer ---
IntrusionAnalyzer::IntrusionAnalyzer(const std::array<float, 2>& line_p1, const std::array<float, 2>& line_p2, const std::vector<std::array<int, 2>>& zone_polygon, float dwell_seconds)
    : line_p1_(line_p1), line_p2_(line_p2), zone_polygon_(zone_polygon), dwell_seconds_(dwell_seconds) {}

std::pair<int, int> IntrusionAnalyzer::bbox_anchor_xyxy(const std::array<float, 4>& xyxy) {
    return {(int)(xyxy[0] + xyxy[2]) / 2, (int)xyxy[3]};
}

float IntrusionAnalyzer::side_of_line(const std::array<float, 2>& p, const std::array<float, 2>& a, const std::array<float, 2>& b) {
    return (b[0] - a[0]) * (p[1] - a[1]) - (b[1] - a[1]) * (p[0] - a[0]);
}

bool IntrusionAnalyzer::is_human_like_motion(int track_id, const std::pair<int, int>& current_center) {
    // Disabled in python: ENABLE_HUMAN_MOTION_FILTER = False
    return true;
}

bool IntrusionAnalyzer::confirm_alarm_condition(int track_id, const std::string& event_type, bool condition) {
    auto key = std::make_pair(track_id, event_type);
    if (condition) {
        alarm_confirm_frames_[key]++;
    } else {
        alarm_confirm_frames_[key] = 0;
    }
    return alarm_confirm_frames_[key] >= ALARM_CONFIRM_FRAMES;
}

bool IntrusionAnalyzer::confirm_line_cross(int track_id, bool crossed, const std::string& direction, bool still_valid) {
    auto key = std::make_pair(track_id, std::string("line_cross"));
    if (crossed && !direction.empty() && still_valid) {
        pending_line_direction[track_id] = direction;
        alarm_confirm_frames_[key] = 1;
        return false;
    }

    if (pending_line_direction.find(track_id) != pending_line_direction.end()) {
        if (still_valid) {
            alarm_confirm_frames_[key]++;
        } else {
            alarm_confirm_frames_[key] = 0;
            pending_line_direction.erase(track_id);
            return false;
        }

        if (alarm_confirm_frames_[key] >= ALARM_CONFIRM_FRAMES) {
            alarm_confirm_frames_[key] = 0;
            return true;
        }
    }
    return false;
}

TrackState IntrusionAnalyzer::update_track(int track_id, const std::pair<int, int>& center, double now, bool box_in_zone_now) {
    if (trails[track_id].size() >= TRAIL_LEN) trails[track_id].pop_front();
    trails[track_id].push_back(center);

    std::array<float, 2> current_p = {(float)center.first, (float)center.second};
    bool is_human_motion = is_human_like_motion(track_id, center);

    bool crossed = false;
    std::string direction = "";

    if (prev_centers_.find(track_id) != prev_centers_.end()) {
        std::array<float, 2> prev_p = {(float)prev_centers_[track_id].first, (float)prev_centers_[track_id].second};
        float s1 = side_of_line(prev_p, line_p1_, line_p2_);
        float s2 = side_of_line(current_p, line_p1_, line_p2_);

        if (s1 == 0) s1 = 1e-6;
        if (s2 == 0) s2 = 1e-6;

        if (s1 * s2 < 0) {
            crossed = true;
            direction = (s1 < 0 && s2 > 0) ? "enter" : "leave";
            line_cross_count++;
        }
    }

    bool entered_zone = false;
    if (box_in_zone_now) {
        zone_in_frames_[track_id]++;
        zone_out_frames_[track_id] = 0;
    } else {
        zone_out_frames_[track_id]++;
        zone_in_frames_[track_id] = 0;
    }

    if (box_in_zone_now && !zone_state_[track_id] && zone_in_frames_[track_id] >= 1) {
        zone_state_[track_id] = true;
        in_zone_since_[track_id] = now;
        entered_zone = true;
        zone_intrusion_count++;
    } else if (!box_in_zone_now && zone_state_[track_id] && zone_out_frames_[track_id] >= MAX_LOST) {
        zone_state_[track_id] = false;
        in_zone_since_.erase(track_id);
    }

    bool in_zone = zone_state_[track_id];
    bool dwell_alarm = false;
    float dwell_time = 0.0f;

    if (in_zone) {
        double since = in_zone_since_.count(track_id) ? in_zone_since_[track_id] : now;
        dwell_time = now - since;
        if (dwell_time >= dwell_seconds_) {
            dwell_alarm = true;
        }
    }

    prev_centers_[track_id] = center;

    return {crossed, direction, in_zone, entered_zone, dwell_alarm, dwell_time, is_human_motion};
}

void IntrusionAnalyzer::clear_track(int track_id) {
    prev_centers_.erase(track_id);
    trails.erase(track_id);
    in_zone_since_.erase(track_id);
    zone_state_.erase(track_id);
    zone_in_frames_.erase(track_id);
    zone_out_frames_.erase(track_id);
    non_human_motion_frames_.erase(track_id);
    pending_line_direction.erase(track_id);

    alarm_confirm_frames_.erase({track_id, "line_cross"});
    alarm_confirm_frames_.erase({track_id, "zone_intrusion"});
    alarm_confirm_frames_.erase({track_id, "dwell_alarm"});
}
