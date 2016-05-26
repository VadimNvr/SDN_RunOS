#pragma once

#include "vector"
#include "list"
#include "chrono"
#include "cstdio"
#include "Switch.hh"
#include <iostream>
#include <fstream>

using std::vector;
using std::list;
using std::pair;
using std::make_pair;

typedef std::chrono::high_resolution_clock::time_point hr_clock_t;

struct SwitchLink {
	size_t s1;
	size_t s2;

	SwitchLink(): s1(0), s2(0) {}
	SwitchLink(size_t s1, size_t s2): s1(s1), s2(s2) {}
};

struct ObservableLink {
	Switch *src;
	Switch *dst;

	uint src_port;
	uint dst_port;

	ObservableLink(): src(nullptr), dst(nullptr) {}
};

struct TimeStamp {
	uint32_t id;
	hr_clock_t time;

	TimeStamp(uint32_t id, hr_clock_t time): id(id), time(time) {}
};

struct TimeSegment {
	hr_clock_t l;
	hr_clock_t r;

	double latency;
	int count;

	TimeSegment(hr_clock_t l, hr_clock_t r, double lat, int cnt=1): l(l), r(r), latency(lat), count(cnt) {}
};

class TimeRequestLoop {
	uint32_t size;
	uint32_t ptr;
	bool closed;
	hr_clock_t *loop;

public:
	TimeRequestLoop(uint32_t size);

	pair<size_t, hr_clock_t> Request();
	hr_clock_t get(uint32_t idx) const;
	hr_clock_t *getLatest() const;
};

class TimeLine {
	list<TimeSegment> line;

public:
	void add(TimeSegment &seg);
	void refresh(hr_clock_t *latest, std::ofstream &out);
	friend class TimeManager;
};

