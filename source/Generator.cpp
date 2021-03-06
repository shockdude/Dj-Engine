#include "Generator.h"

//read little endian int from data stream
void readToInt(std::ifstream& stream, int* i) {
	char c;
	char* mod = (char*)i;
	for (int i = 0; i < 4; i++) {
		stream.read(&c, sizeof(c));
		mod[3 - i] = c;
	}
}

//read little endian float from data stream
void readToFloat(std::ifstream& stream, float* f) {
	char c;
	char* mod = (char*)f;
	for (int i = 0; i < 4; i++) {
		stream.read(&c, sizeof(c));
		mod[3 - i] = c;
	}
}

Generator::Generator() {
}

void Generator::init(SongEntry entry) {
	m_songEntry = entry;
	m_allCross.push_back(Note(m_initialCrossfade, CROSS_C, 0.0f, true));//Do not remove
	m_bpm = entry.bpm;
	std::string textPath = entry.path + std::string("/chart.txt");
	m_chart.open(textPath);
	if (m_chart.is_open()) {
		//write chart data to console
		m_isChartBinary = false;
		std::cout << "Generator msg: loaded text chart" << std::endl;
		std::string version;
		m_chart >> version;
		std::cout << "Generator msg: Chart Version: " << version << std::endl;
		initialLoad();
	}
	else {
		std::cout << "Generator msg: text chart not found, opening fgsmub" << std::endl;
		std::string chartPath = entry.path + std::string("/chart.fsgmub");
		m_chart.open(chartPath, std::ios::binary);
		if (m_chart.is_open()) {
			//write chart data to console
			m_isChartBinary = true;
			std::cout << "Generator msg: loaded fgsmub chart" << std::endl;

			int version = 0;
			int dummy = 0;
			readToInt(m_chart, &version);
			readToInt(m_chart, &dummy);
			readToInt(m_chart, &dummy);
			readToInt(m_chart, &dummy);

			std::cout << "version: " << version << std::endl;
			initialLoad();
		}
		else {
			std::cout << "Generator msg: error loading fsgmub file, opening xmk file" << std::endl;
			std::string chartPath = entry.path + std::string("/chart.xmk");
			m_chart.open(chartPath, std::ios::binary);
			if (m_chart.is_open()) {
				//write chart data to console
				m_isChartBinary = true;
				std::cout << "loaded xmk chart" << std::endl;

				int version = 0;
				int dummy = 0;
				readToInt(m_chart, &version);
				readToInt(m_chart, &dummy);
				readToInt(m_chart, &dummy);
				readToInt(m_chart, &dummy);

				std::cout << "version: " << version << std::endl;
			}
			else {
				std::cerr << "Generator Error: could not load chart file" << std::endl;
			}
			initialLoad();
		}
	}
}

void Generator::initialLoad(){
	/*
	read the chart IF there are less than 100 notes
	and less than 100 events on screen.

	This limit is arbitrary and can change in the future.
	Mostly it's there to use less amount of ram on the pc

	(but really, 100 notes present on screen is ridiculous)
	*/
	if (m_isChartBinary) {
		while (!m_chart.eof()) {
			float time;
			int type;
			float length = 0.0;
			float extra;

			//read entry from file
			readToFloat(m_chart, &time);
			readToInt(m_chart, &type);
			readToFloat(m_chart, &length);
			readToFloat(m_chart, &extra);

			double factor = 240.0 / m_bpm;

			time *= (float)factor;
			length *= (float)factor;

			//std::cout << time << "\t" << type << "\t" << length << "\t" << extra << std::endl;

			//decode type from entry
			if (type == 0) {
				m_allTaps.push_back(Note(time, TAP_G, 0.0, false));
			}
			else if (type == 1) {
				m_allTaps.push_back(Note(time, TAP_B, 0.0, false));
			}
			else if (type == 2) {
				m_allTaps.push_back(Note(time, TAP_R, 0.0, false));
			}
			else if (type == 3) {
				m_allTaps.push_back(Note(time, SCR_G_UP, 0.0, false));
			}
			else if (type == 4) {
				m_allTaps.push_back(Note(time, SCR_B_UP, 0.0, false));
			}
			else if (type == 5) {
				m_allTaps.push_back(Note(time, SCR_G_DOWN, 0.0, false));
			}
			else if (type == 6) {
				m_allTaps.push_back(Note(time, SCR_B_DOWN, 0.0, false));
			}
			else if (type == 7) {
				m_allTaps.push_back(Note(time, SCR_G_ANY, length, false));
				
				float end = time + length;
				float betweenTicks = 60.0f / ((float)m_bpm * TICKS_PER_BEAT);
				time += betweenTicks;
				while (time < end) {
					m_allTaps.push_back(Note(time, SCR_G_TICK, length, false));
					time += betweenTicks;
				}
				
			}
			else if (type == 8) {
				m_allTaps.push_back(Note(time, SCR_B_ANY, length, false));
				float end = time + length;
				float betweenTicks = 60.0f / ((float)m_bpm * TICKS_PER_BEAT);
				time += betweenTicks;
				while (time < end) {
					m_allTaps.push_back(Note(time, SCR_B_TICK, length, false));
					time += betweenTicks;
				}
				
			}
			else if (type == 9) {
				m_allCross.push_back(Note(time, CROSS_B, 0.0, true));
			}
			else if (type == 10) {
				if (time > 0.0) {
					m_allCross.push_back(Note(time, CROSS_C, 0.0, true));
				}
			}
			else if (type == 11) {
				m_allCross.push_back(Note(time, CROSS_G, 0.0, true));
			}
			else if (type == 15) {
				m_allEvents.push_back(Note(time, EU_ZONE, length, true));
			}
			else if (type == 20) {
				m_allEvents.push_back(Note(time, SCR_G_ZONE, length, true));
			}
			else if (type == 21) {
				m_allEvents.push_back(Note(time, SCR_B_ZONE, length, true));
			}
			else if (type == 27) {
				m_allTaps.push_back(Note(time, CF_SPIKE_G, 0.0, false));
				//pushNote((double)time, CF_SPIKE_G, 0.0);
			}
			else if (type == 28) {
				m_allTaps.push_back(Note(time, CF_SPIKE_B, 0.0, false));
				//pushNote((double)time, CF_SPIKE_B, 0.0);
			}
			else if (type == 29) {
				m_allTaps.push_back(Note(time, CF_SPIKE_C, 0.0, false));
				//pushNote((double)time, CF_SPIKE_C, 0.0);
			}
			else if (type == 48) {
				m_allEvents.push_back(Note(time, SCR_G_ZONE, length, true));

				//pushEvent((double)time, SCR_G_ZONE, (double)length);
			}
			else if (type == 49) {
				m_allEvents.push_back(Note(time, SCR_B_ZONE, length, true));

				//pushEvent((double)time, SCR_B_ZONE, (double)length);
			}
			else if (type == 0x0B000001) {
				//bpm marker
			}
			else if (type == 0x0B000002) {
				m_bpm = extra;
			}
			else if (type == 0x0AFFFFFF) {}
			else if (type == 0xFFFFFFFF) {
				//chart start
			}
			//else std::cerr << "error parsing entry: " << time << "\t"<< type << "\t"<< length << "\t"<< extra<<std::endl;
		}
		if (m_chart.eof() && !m_placedFinalCF) {
			m_placedFinalCF = true;

			//place a crossfade center at the 100 hour mark
			//seems impossible to reach it for me 
			//but then, there is always someone that does the impossible
			//pushCross(360000.0f, CROSS_C, 0.0f);
			m_allCross.push_back(Note(360000.0, CROSS_C, 0.0f, true));
		}
	}
}

//update notes/events every frame
void Generator::tick(double time, std::vector<Note>& v, std::vector<Note>& ev, std::vector<Note>& cross) {
	m_combo_reset = false;
	m_eu_check = false;
	/*
	size_t note_s = m_note_times.size();

	//note generation from 'cache' (m_note_times)
	for (size_t i = 0; i < note_s; i++) {
		double time = m_note_times.at(0);
		int type = m_note_types.at(0);
		double length = m_note_length.at(0);

		Note temp(time, type, length, false);
		v.push_back(temp);
		m_note_times.erase(m_note_times.begin());
		m_note_types.erase(m_note_types.begin());
		m_note_length.erase(m_note_length.begin());
	}
	size_t event_s = m_event_times.size();

	//event generation from 'cache' (m_event_times)
	for (size_t i = 0; i < event_s; i++) {
		double time = m_event_times.at(0);
		int type = m_event_types.at(0);
		double length = m_event_length.at(0);
		Note e(time, type, length, true);
		ev.push_back(e);
		m_event_times.erase(m_event_times.begin());
		m_event_types.erase(m_event_types.begin());
		m_event_length.erase(m_event_length.begin());
	}

	size_t cross_s = m_cross_times.size();
	//crossfade generation from 'cache' (m_cross_times)
	for (size_t i = 0; i < cross_s; i++) {
		double time = m_cross_times.at(0);
		int type = m_cross_types.at(0);
		double length = m_cross_length.at(0);
		Note c(time, type, length, true);
		cross.push_back(c);
		m_cross_times.erase(m_cross_times.begin());
		m_cross_types.erase(m_cross_types.begin());
		m_cross_length.erase(m_cross_length.begin());
	}
	*/

	if (!v.empty()) {
		for (size_t i = v.size(); i-- > 0;) {
			v.at(i).tick(time);
			int type = v.at(i).getType();
			//remove if outside hit area
			if (v.at(i).getDead()) {
				if (v.at(i).getTouched()) {
					if (type == SCR_G_TICK || type == SCR_B_TICK || type == SCR_G_ANY || type == SCR_B_ANY) {
						m_notesTotal--;
						m_notesHit--;
						m_scr_tick++;
					}
					m_notesHit++;
				}
				else {
					m_combo_reset = true;
				}
				m_notesTotal++;
				v.erase(v.begin() + i);
			}
		}
	}

	if (!ev.empty()) {
		for (size_t i = ev.size(); i-- > 0;) {
			ev.at(i).tick(time);
			int type = ev.at(i).getType();

			/*
			every event has a different way to be removed
			for example: a scratch start cannot be removed
			if the corrisponding scratch end is before the clicker
			*/

			if (type == SCR_G_ZONE || type == SCR_B_ZONE) {
				double endTime = ev.at(i).getMilli() + ev.at(i).getLength();
				if (endTime < time - ev.at(i).hitWindow) {
					ev.erase(ev.begin() + i);
				}
			}
			if (type == EU_ZONE) {
				//start eu zone check when the event is on the clicker
				if (ev.at(i).getHit() && !ev.at(i).getTouched()) {
					m_eu_start = true;
					ev.at(i).click(time);
				}
				double endTime = ev.at(i).getMilli() + ev.at(i).getLength();
				//set signal for player
				if (endTime < time) {
					m_eu_check = true;
					ev.erase(ev.begin() + i);
				}
				else if (m_combo_reset) {
					//remove event if misclick
					if (ev.at(i).getMilli() + ev.at(i).hitWindow < time) {
						ev.erase(ev.begin() + i);
					}
				}
			}
		}
	}

	if (!cross.empty()) {
		for (size_t i = 0; i < cross.size() - 1; i++) {
			cross.at(i).tick(time);

			double next_time = cross.at(i + 1).getMilli();
			//if the next crossfader has crossed the clickers
			if (next_time + 0.15 <= time) {
				if (cross.at(i).getTouched()) {
					m_notesHit++;
				}
				else {
					m_combo_reset = true;
				}
				m_notesTotal++;
				cross.erase(cross.begin() + i);
			}
			if (cross.at(i).getMilli() == m_initialCrossfade) {
				cross.at(i).setTouched(true);
			}
		}
		cross.at(cross.size() - 1).tick(time);
	}

	if (m_scr_tick > 4) {
		m_scr_tick -= 4;
		m_notesHit++;
		m_notesTotal++;
	}

	/*
	if (m_bpmChangeTime != -1 && m_bpmChangeValue != -1 && time + 1.0 >= m_bpmChangeTime) {
		m_bpm = m_bpmChangeValue;
		m_bpmChangeValue = -1;
		m_bpmChangeTime = -1;
	}
	*/
}

/*
void Generator::textParser(std::vector<Note>& v, std::vector<Note>& ev) {
	read the chart IF there are less than 100 notes
	and less than 100 events on screen.

	This limit is arbitrary and can change in the future.
	Mostly it's there to use less amount of ram on the pc

	(but really, 100 notes present on screen is ridiculous)

	if (!m_isChartBinary) {
		size_t noteBufferSize = v.size() + m_note_times.size();
		size_t eventBufferSize = ev.size() + m_event_times.size();

		while (!m_chart.eof() && noteBufferSize < 100 && eventBufferSize < 100) {
			std::string token;
			double t;
			m_chart >> token;

			/*
			depending by the tokens, add note/event to 'cache'


			if (token == "T" || token == "t") {
				m_chart >> token;
				if (token == "R" || token == "r") {
					m_chart >> token;
					t = std::stod(token);
					pushNote(t, TAP_R, 0.0);
				}
				else if (token == "G" || token == "g") {
					m_chart >> token;
					t = std::stod(token);
					pushNote(t, TAP_G, 0.0);
				}
				else if (token == "B" || token == "b") {
					m_chart >> token;
					t = std::stod(token);
					pushNote(t, TAP_B, 0.0);
				}
				else {
					std::cerr << "error parsing token:T " << token << std::endl;
				}
			}
			else if (token == "C" || token == "c") {
				m_chart >> token;
				if (token == "G" || token == "g") {
					m_chart >> token;
					t = std::stod(token);
					pushEvent(t, CROSS_G, 0.0);
				}
				else if (token == "B" || token == "b") {
					m_chart >> token;
					t = std::stod(token);
					pushEvent(t, CROSS_B, 0.0);
				}
				else if (token == "C" || token == "c") {
					m_chart >> token;
					t = std::stod(token);
					pushEvent(t, CROSS_C, 0.0);
				}
				else {
					std::cerr << "error parsing token:C " << token << std::endl;
				}
			}
			else if (token == "S" || token == "s") {
				m_chart >> token;
				if (token == "G" || token == "g") {
					m_chart >> token;
					if (token == "U" || token == "u") {
						m_chart >> token;
						t = std::stod(token);
						pushNote(t, SCR_G_UP, 0.0);
					}
					else if (token == "D" || token == "d") {
						m_chart >> token;
						t = std::stod(token);
						pushNote(t, SCR_G_DOWN, 0.0);
					}
					else if (token == "A" || token == "a") {
						m_chart >> token;
						t = std::stod(token);
						pushNote(t, SCR_G_ANY, 0.0);
					}
					else if (token == "S" || token == "s") {
						m_chart >> token;
						t = std::stod(token);
						double length = 0.0;
						m_chart >> token;
						length = std::stod(token);
						pushEvent(t, SCR_G_ZONE, length);
					}
					else {
						std::cerr << "error parsing token:S G " << token << std::endl;
					}
				}
				else if (token == "B") {
					m_chart >> token;
					if (token == "U" || token == "u") {
						m_chart >> token;
						t = std::stod(token);
						pushNote(t, SCR_B_UP, 0.0);
					}
					else if (token == "D" || token == "d") {
						m_chart >> token;
						t = std::stod(token);
						pushNote(t, SCR_B_DOWN, 0.0);
					}
					else if (token == "A" || token == "a") {
						m_chart >> token;
						t = std::stod(token);
						pushNote(t, SCR_B_ANY, 0.0);
					}
					else if (token == "S" || token == "s") {
						m_chart >> token;
						t = std::stod(token);
						double length = 0.0;
						m_chart >> token;
						length = std::stod(token);
						pushEvent(t, SCR_B_ZONE, length);
					}
					else {
						std::cerr << "error parsing token:S B " << token << std::endl;
					}
				}
				else {
					std::cerr << "error parsing token:S " << token;
				}
			}
			else if (token == "E" || token == "e") {
				m_chart >> token;
				if (token == "S" || token == "s") {
					m_chart >> token;
					t = std::stod(token);
					double length = 0.0;
					m_chart >> token;
					length = std::stod(token);
					pushEvent(t, EU_ZONE, length);
				}
				else {
					std::cerr << "error parsing token:E " << token << std::endl;
				}
			}
			else if (token == "BPM" || token == "bpm") {
				m_chart >> token;
				m_bpmChangeTime = std::stod(token);
				m_chart >> token;
				m_bpmChangeValue = std::stoi(token);
			}
			else if (token == "SET" || token == "set") {
				m_chart >> token;
				if (token == "TIME" || token == "time") {
					m_chart >> token;
					if (token == "ABS" || token == "abs") m_isChartBinary = false;
					else if (token == "REL" || token == "rel") m_isChartBinary = true;
					else std::cerr << "error parsing token:SET TIME" << std::endl;
				}
				else std::cerr << "error parsing token:SET" << std::endl;
			}
			else {
				std::cerr << "unknown token:" << token << std::endl;
			}

			noteBufferSize = v.size() + m_note_times.size();
			eventBufferSize = ev.size() + m_event_times.size();
		}
	}
}
*/

void Generator::addNotesToBuffer(std::vector<Note>& v, std::vector<Note>& ev, std::vector<Note>& cross) {
	while (v.size() < 100 && m_allTaps.size() > 0) {
		v.push_back(m_allTaps.front());
		m_allTaps.pop_front();
	}
	while (ev.size() < 100 && m_allEvents.size() > 0) {
		ev.push_back(m_allEvents.front());
		m_allEvents.pop_front();
	}
	while (cross.size() < 100 && m_allCross.size() > 0) {
		cross.push_back(m_allCross.front());
		m_allCross.pop_front();
	}
}

void Generator::bpm(double time, std::vector<double>& arr) {
	//update bpm tick array

	for (size_t i = 0; i < arr.size(); i++) {
		if (arr.at(i) < time - 0.2) {
			arr.erase(arr.begin() + i);
		}
	}
	double nextTick = m_lastBpmTick + ((double)60 / m_bpm);
	while (time + m_deckSpeed >= nextTick) {
		arr.push_back(nextTick);
		m_lastBpmTick = nextTick;
		nextTick = m_lastBpmTick + ((float)60 / m_bpm);
	}
}

void Generator::reset() {
	m_chart.close();
	m_isChartBinary = false;
	m_placedFinalCF = false;
	m_bpmChangeTime = -1;
	m_bpmChangeValue = -1;
	m_lastBpmTick = 0.0;
	m_notesHit = 0;
	m_notesTotal = 0;

	m_allTaps.clear();
	m_allEvents.clear();
	m_allCross.clear();

	m_combo_reset = false;
	m_eu_start = false;
	m_eu_check = false;
}

int Generator::getNotesTotal() {
	return m_notesTotal;
}

int Generator::getNotesHit() {
	return m_notesHit;
}

SongEntry Generator::getSongEntry()
{
	return m_songEntry;
}

//utility functions 

Generator::~Generator() {
	m_chart.close();
	//dtor
}
