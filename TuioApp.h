#include "TuioListener.h"
#include "TuioClient.h"
#include "TuioObject.h"
#include <list>

class TuioApp : public TuioListener
{
	public:
		TuioApp();
		~TuioApp();

		// these TUIO funcs are called asynchronously by Client thread
		void addTuioObj(unsigned int s_id, unsigned int f_id);
		void updateTuioObj(unsigned int s_id, unsigned int f_id, float xpos, float ypos, float angle, float x_speed, float y_speed, float r_speed, float m_accel, float r_accel);
		void removeTuioObj(unsigned int s_id, unsigned int f_id);
		void refresh();

	private:
		TuioClient *client;

		void startClient(int port);
		void stopClient();

		std::list<TuioObject> objectList;
		std::list<TuioObject>::iterator tuioObject;
};
