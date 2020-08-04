#ifndef TIMER_HPP
#define TIMER_HPP
class Timer{
public:
	bool operator()(){
		if (m_pause_for == 0){
			if (m_time_left > 0){
				m_time_left--;
			}
		} else {
			m_pause_for--;
		}
		return ((m_pause_for == 0)&&(m_time_left == 0));
	}
	int getDelay() const{
		return m_pause_for;
	}
	int getTime() const{
		return m_time_left;
	}
	void setTime(int time){
		m_time_left = time;
	}
	void setDelay(int delay){
		m_pause_for = delay;
	}
private:
	int m_pause_for = 0;
	int m_time_left = 0;
};
#endif
