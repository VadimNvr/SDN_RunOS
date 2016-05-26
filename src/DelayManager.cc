#include "DelayManager.hh"

REGISTER_APPLICATION(DelayManager, {"switch-manager", "static-flow-pusher", ""})

void DelayManager::init(Loader* loader, const Config& config) {
	cycles = 1000;

	switchManager = SwitchManager::get(loader);
    flowPusher    = StaticFlowPusher::get(loader);
}

void DelayManager::observeLink(switch_and_port from, switch_and_port to) {

	std::cout << "DelayManager::Trying to observe link\n";
	if (addLinkMutex.try_lock()) {
		std::cout << "DelayManager::Observing link\n";
		size_t link = addLink(from, to);
		char file_name[32];
		sprintf(file_name, "s%lup%u-s%lup%u.txt", from.dpid, from.port, to.dpid, to.port);
		delay_logs[link].open(file_name, std::ofstream::out);
    	std::thread adLink(&DelayManager::launchObserver, this, from, to, link);
    	adLink.detach();
    }
    else {
    	std::cout << "DelayManager::Can't observe link now\n";
    }
}

void DelayManager::launchObserver(switch_and_port from, switch_and_port to, size_t link) {
	loadEthRules(link);
	addLinkMutex.unlock();
}

void DelayManager::loadEthRules(size_t link_idx) 
{
	std::cout << "Loading rules...\n";
	dm::Link *link = links[link_idx];

	Switch *s1 = switchManager->getSwitch(link->from.dpid);
    Switch *s2 = switchManager->getSwitch(link->to.dpid);

    char ethSrc[17];
    FlowDesc test_fd;
    test_fd.eth_dst("10:ff:ff:ff:ff:ff");
    test_fd.priority(10000);
    test_fd.out_port(link->from.port);
    flowPusher->sendToSwitch(s1, &test_fd); 

    //++EthDst
    for (size_t i = 1; i <= cycles; ++i) {
        FlowDesc fd;
        getEth(link_idx, i, ethSrc);

        fd.eth_src(ethSrc);
        fd.in_port(link->from.port);
        fd.eth_type(LLDP_ETH_TYPE);
        fd.priority(1000);

        getEth(link_idx, i+1, ethSrc);
        fd.modifyField(new of13::EthSrc(ethSrc));
        fd.out_port(of13::OFPP_IN_PORT);
        flowPusher->sendToSwitch(s1, &fd); 
    }

    //RETURN RULE
    FlowDesc fd_return;
    fd_return.eth_type(LLDP_ETH_TYPE);
    fd_return.in_port(link->to.port);
    fd_return.out_port(of13::OFPP_IN_PORT);
    fd_return.priority(1);
    flowPusher->sendToSwitch(s2, &fd_return); 
}

size_t DelayManager::addLink(switch_and_port &from, switch_and_port &to) 
{
	std::cout << "DelayManager::Adding link...\n";
	WriteLock w_link_lock(linksLock);
	dm::Link *link = new dm::Link(from, to);
	size_t idx = links.add(link);

	link_ids[from.dpid][from.port] = idx;
	link_ids[to.dpid][to.port] = idx;

	std::cout << "DelayManager::Adding loop...\n";
	WriteLock w_loop_lock(loopLock[idx]);
	loops[idx] = new TimeRequestLoop(12);
	timelines[idx] = new TimeLine();

	return idx;
}

void DelayManager::addLoop(size_t link_idx) 
{
	loops[link_idx] = new TimeRequestLoop(1024);
}

void DelayManager::addTimeLine(size_t link_idx)
{
	timelines[link_idx] = new TimeLine();
}

void DelayManager::onDmpReceived(unsigned short link_id, unsigned pkt_id, unsigned long long t0, unsigned long long t1)
{
	std::chrono::high_resolution_clock::time_point start, end;
	memcpy(&start, &t0, sizeof t0);
	memcpy(&end,   &t1, sizeof t1);
	std::cout << "Dmp!\n";

	if (true) {
		ReadLock r_lock(loopLock[link_id]);

		if (loops[link_id]->get((uint32_t) pkt_id) != start) {
			std::cout << "Dropped" << std::endl;
			return;
		}
	}

	std::chrono::duration<double, std::milli> delta = end - start;
	TimeSegment seg(start, end, delta.count()/cycles);
	//std::cout << delta.count()/cycles << std::endl;
	//char msg[512];
	//sprintf(msg, "%lf", delta.count()/cycles);
	//links[link_id]->write(msg);
	timeLineMutex[link_id].lock();
	timelines[link_id]->add(seg);
	timelines[link_id]->refresh(loops[link_id]->getLatest(), delay_logs[link_id]);
	timeLineMutex[link_id].unlock();
	//std::cout << "onDmp! LINK is " << link_id << ", ID is " << pkt_id << ", Time is " << delta.count()/cycles << "ms" << std::endl;
}

template <typename T>
template <typename C>
size_t std::RingArray<T>::add(const C *elem) {
	if (array.size() < max_size) {
		array.push_back(elem);
		ptr = (ptr+1) % max_size;
		return ptr ? ptr-1 : max_size-1;
	}

	if (rewrite) {
		delete array[ptr];
		array[ptr] = elem;
		ptr = (ptr+1) % max_size;
	}
	else {
		size_t last = ptr ? ptr-1 : max_size-1;

		for (size_t i = ptr; i != last; i = (i+1) % max_size) {
			if (array[i] == nullptr) {
				array[i] = elem;
				ptr = (i+1) % max_size;
				return i;
			}
		}

		throw "No space availible in array";
	}
}

template <typename T>
size_t std::RingArray<T>::add(const T &elem) {
	if (array.size() < max_size) {
		array.push_back(elem);
		ptr = (ptr+1) % max_size;
		return ptr ? ptr-1 : max_size-1;
	}

	if (rewrite) {
		array[ptr] = elem;
		ptr = (ptr+1) % max_size;
		return ptr ? ptr-1 : max_size-1;
	}
	else {
		size_t last = ptr ? ptr-1 : max_size-1;

		for (size_t i = ptr; i != last; i = (i+1) % max_size) {
			if (array[i] == 0) {
				array[i] = elem;
				ptr = (i+1) % max_size;
				return i;
			}
		}

		throw "No space availible in array";
	}
}

template <typename T>
void std::RingArray<T>::remove(const size_t idx) {
	if (std::is_pointer<T>::value) {
		delete array[idx];
		array[idx] = nullptr;
	}
}

template <typename T>
T std::RingArray<T>::get(const size_t idx) const{
	return array[idx];
}

template <typename T>
T std::RingArray<T>::operator[](const size_t idx) const{
	return array[idx];
}

long DelayManager::getLinkIdBySwitch(switch_and_port &src) {
	ReadLock r_lock(linksLock);
	if EXIST2(link_ids, src.dpid, src.port) {
		return link_ids[src.dpid][src.port];
	}
	else {
		return -1;
	}
}

dm::Link *DelayManager::getLinkById(size_t id) 
{
	ReadLock r_lock(linksLock);
	return links.get(id);
}

pair<size_t, hr_clock_t> DelayManager::requestPacketOut(size_t link) 
{
	WriteLock w_lock(loopLock[link]);
	auto res = loops[link]->Request();

	return res;
}

void getIpForCycle(uint32_t cycle, char *buf) {

    uint8_t addr[] = {
        (uint8_t) (cycle / 256 / 256 / 256 % 256),
        (uint8_t) (cycle / 256 / 256 % 256),
        (uint8_t) (cycle / 256 % 256),
        (uint8_t) (cycle % 256)
    };

    sprintf(buf, "%u.%u.%u.%u", addr[0], addr[1], addr[2], addr[3]);
}

void getEthForSwitchPair(size_t s1, size_t s2, char *buf) {

	uint8_t a[] = {
		(uint8_t) (s1 / 256 / 256 % 256),
		(uint8_t) (s1 / 256 % 256), 
		(uint8_t) (s1 % 256)
	};

	uint8_t b[] = {
		(uint8_t) (s2 / 256 / 256 % 256), 
		(uint8_t) (s2 / 256 % 256),
		(uint8_t) (s2 % 256)
	};

	sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", a[0], a[1], a[2], b[0], b[1], b[2]);
}

void getEth(uint16_t link_idx, uint32_t cycle, char *buf) {

    uint8_t addr[] = {
        (uint8_t) (cycle / 256 / 256 / 256 % 256),
        (uint8_t) (cycle / 256 / 256 % 256),
        (uint8_t) (cycle / 256 % 256),
        (uint8_t) (cycle % 256)
    };

    uint8_t link[] = {
    	(uint8_t) (link_idx / 256 % 256),
    	(uint8_t) (link_idx % 256)
    };

    sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", link[0], link[1], addr[0], addr[1], addr[2], addr[3]);
}