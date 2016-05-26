#include "TimeUtils.hh"
#define mean(a, b) ((a + b) / 2)


/*
TIME REQUEST LOOP
*/
TimeRequestLoop::TimeRequestLoop(uint32_t size): size(size), ptr(0), closed(false) {
	 loop = new hr_clock_t[size];
}

pair<size_t, hr_clock_t> TimeRequestLoop::Request() {
	if (ptr == size-1)
		closed = true;

	hr_clock_t now = std::chrono::high_resolution_clock::now();
	loop[ptr] = now;

	ptr = (ptr + 1) % size;

	return make_pair(ptr ? ptr-1 : size-1, now);
}

hr_clock_t TimeRequestLoop::get(uint32_t idx) const {
	return loop[idx % size];
}

hr_clock_t *TimeRequestLoop::getLatest() const {
	if (!closed) {
		if (ptr == 0)
			return nullptr;
		else
			return loop;
	}
	else {
		return loop + (ptr + 1) % size;
	}
}

void TimeLine::add(TimeSegment &newSeg) {

	for (auto seg = line.rbegin(); seg != line.rend(); ++seg) {
		//TimeSegment &seg = *idx;

		if (newSeg.l >= seg->r) {
			line.insert(seg.base(), newSeg);
			return;
		}
		if (newSeg.r > seg->r) {
			if (newSeg.l > seg->l) {
				line.insert(seg.base(), TimeSegment(seg->r, newSeg.r, newSeg.latency));
				++seg;
				line.insert(seg.base(), TimeSegment(newSeg.l, seg->r, seg->latency+newSeg.latency, seg->count+1));
				++seg;
				seg->r = newSeg.l;
				return;
			}
			if (newSeg.l == seg->l) {
				line.insert(seg.base(), TimeSegment(seg->r, newSeg.r, newSeg.latency));
				++seg;
				seg->latency += newSeg.latency;
				seg->count += 1;
				return;
			}
			if (newSeg.l < seg->l) {
				line.insert(seg.base(), TimeSegment(seg->r, newSeg.r, newSeg.latency));
				++seg;
				seg->latency += newSeg.latency;
				seg->count += 1;
				newSeg.r = seg->l;
			}
		}
		else if (newSeg.r == seg->r) {
			if (newSeg.l > seg->l) {
				line.insert(seg.base(), TimeSegment(newSeg.l, newSeg.r, seg->latency+newSeg.latency, seg->count+1));
				++seg;
				seg->r = newSeg.l;
				return;
			}
			if (newSeg.l == seg->l) {
				seg->latency += newSeg.latency;
				seg->count += 1;
				return;
			}
			if (newSeg.l < seg->l) {
				seg->latency += newSeg.latency;
				seg->count += 1;
				newSeg.r = seg->l;
			}
		}
	}

	line.push_back(newSeg);
}

void TimeLine::refresh(hr_clock_t *latest, std::ofstream &out) {
	if (!latest)
		return;

	while (line.front().r <= *latest) {
		auto t = std::chrono::time_point_cast<std::chrono::milliseconds>(line.front().l).time_since_epoch().count() +
				 std::chrono::time_point_cast<std::chrono::milliseconds>(line.front().r).time_since_epoch().count();
		t /= 2;
		out << t << ' ' << line.front().latency / line.front().count << std::endl;
		std::cout << line.front().count << std::endl;
		line.pop_front();
	}
}
