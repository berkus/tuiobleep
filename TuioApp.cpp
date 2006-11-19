#include <assert.h>
#include <math.h>
#include <iostream>
#include <jack/jack.h>
#include <jack/transport.h>

#include "TuioApp.h"

using namespace std;

#define TWOPI (float)(2.0*M_PI)

// jack driver & thread

class JackAudioDriver
{
	public:
		JackAudioDriver();
		~JackAudioDriver();

	// These are the static versions of the callbacks, they call
	// the non-static ones below
	inline static int  process_cb(jack_nframes_t nframes, void* const jack_driver);
	inline static void shutdown_cb(void* const jack_driver);
	inline static int  buffer_size_cb(jack_nframes_t nframes, void* const jack_driver);
	inline static int  sample_rate_cb(jack_nframes_t nframes, void* const jack_driver);

	// Non static callbacks
	int  m_process_cb(jack_nframes_t nframes);
	void m_shutdown_cb();
	int  m_buffer_size_cb(jack_nframes_t nframes);
	int  m_sample_rate_cb(jack_nframes_t nframes);


	void generate_sine(jack_nframes_t nframes);


	volatile bool note_on;// = false;
	volatile float pitch;// = 440.0;

	jack_port_t*          m_jack_port;
	jack_client_t*         m_client;
	jack_nframes_t         m_buffer_size;
	jack_nframes_t         m_sample_rate;
	jack_default_audio_sample_t *m_jack_buffer;

	float m_amplitude;
	float m_phase;

	jack_nframes_t m_start_of_current_cycle;
	jack_nframes_t m_start_of_last_cycle;
};

inline int JackAudioDriver::process_cb(jack_nframes_t nframes, void* jack_driver)
{
	return ((JackAudioDriver*)jack_driver)->m_process_cb(nframes);
}

inline void JackAudioDriver::shutdown_cb(void* jack_driver)
{
	return ((JackAudioDriver*)jack_driver)->m_shutdown_cb();
}


inline int JackAudioDriver::buffer_size_cb(jack_nframes_t nframes, void* jack_driver)
{
	return ((JackAudioDriver*)jack_driver)->m_buffer_size_cb(nframes);
}


inline int JackAudioDriver::sample_rate_cb(jack_nframes_t nframes, void* jack_driver)
{
	return ((JackAudioDriver*)jack_driver)->m_sample_rate_cb(nframes);
}

// create driver
JackAudioDriver::JackAudioDriver()
	: note_on(false)
	, pitch(440.0)
	, m_jack_port(0)
	, m_client(0)
	, m_buffer_size(0)
	, m_sample_rate(0)
	, m_jack_buffer(0)
	, m_amplitude(1.0)
	, m_phase(0.0)
{
	m_client = jack_client_new("tuiobleep");
	if (m_client == NULL) {
		cerr << "[JackAudioDriver] Unable to connect to Jack.  Exiting." << endl;
		exit(EXIT_FAILURE);
	}

	jack_on_shutdown(m_client, shutdown_cb, this);

	m_buffer_size = jack_get_buffer_size(m_client);
	m_sample_rate = jack_get_sample_rate(m_client);

	jack_set_sample_rate_callback(m_client, sample_rate_cb, this);
	jack_set_buffer_size_callback(m_client, buffer_size_cb, this);

	// register port
	m_jack_port = jack_port_register(m_client,
		"/", JACK_DEFAULT_AUDIO_TYPE,
		JackPortIsOutput,
		0);

	// prepare buffer
	m_jack_buffer = (jack_default_audio_sample_t*)jack_port_get_buffer(m_jack_port, m_buffer_size);

	// activate driver
	jack_set_process_callback(m_client, process_cb, this);

	if (jack_activate(m_client)) {
		cerr << "[JackAudioDriver] Could not activate Jack client, aborting." << endl;
		exit(EXIT_FAILURE);
	} else {
		cout << "[JackAudioDriver] Activated Jack client." << endl;
	}
}

JackAudioDriver::~JackAudioDriver()
{
// deactivate driver
		jack_deactivate(m_client);

		jack_port_unregister(m_client, m_jack_port);

		cout << "[JackAudioDriver] Deactivated Jack client." << endl;
// destroy driver
		jack_client_close(m_client);
}

// processing
/**** Jack Callbacks ****/

/** Jack process callback, drives entire audio thread.
 *
 * \callgraph
 */
int
JackAudioDriver::m_process_cb(jack_nframes_t nframes)
{
	// FIXME: support nframes != buffer_size, even though that never damn well happens
	assert(nframes == m_buffer_size);

	// Note that jack can elect to not call this function for a cycle, if things aren't
	// keeping up
	m_start_of_current_cycle = jack_last_frame_time(m_client);
	m_start_of_last_cycle = m_start_of_current_cycle - nframes;

	assert(m_start_of_current_cycle - m_start_of_last_cycle == nframes);

//	m_transport_state = jack_transport_query(m_client, &m_position);

//	process_events(m_start_of_last_cycle, m_start_of_current_cycle);
//	om->midi_driver()->prepare_block(m_start_of_last_cycle, m_start_of_current_cycle);

	// Set buffers of patch ports to Jack port buffers (zero-copy processing)
//	for (List<JackAudioPort*>::iterator i = m_ports.begin(); i != m_ports.end(); ++i)
//		(*i)->prepare_buffer(nframes);

	// Run root patch
//	assert(m_root_patch != NULL);
//	m_root_patch->run(nframes);
// 	cout << "jack callback: note on " << note_on << ", pitch " << pitch << endl;
	if(note_on)
	{
		generate_sine(nframes);
	}

	return 0;
}

void
JackAudioDriver::generate_sine(jack_nframes_t nframes)
{
    float phase_inc = pitch / m_sample_rate;
//     float** framedata = (float**)m_jack_buffer;
    for(jack_nframes_t i = 0; i < nframes; i++)
    {
        m_phase += phase_inc;
        if(m_phase > 1.0) m_phase -= 1.0;
        float d = m_amplitude * sinf(m_phase*TWOPI);
        m_jack_buffer[i] = d;
/*        for(int j = 0; j < frame->channels; j++)
        {
            framedata[j][i] = d;
        }*/
    }
}

extern volatile bool quit_flag;

void
JackAudioDriver::m_shutdown_cb()
{
	cout << "[JackAudioDriver] Jack shutdown.  Exiting." << endl;
	quit_flag = true;
}

int
JackAudioDriver::m_sample_rate_cb(jack_nframes_t nframes)
{
//	if (m_is_activated) {
//		cerr << "[JackAudioDriver] Om does not support changing sample rate on the fly (yet).  Aborting." << endl;
//		exit(EXIT_FAILURE);
//	} else {
		m_sample_rate = nframes;
//	}
	return 0;
}

int
JackAudioDriver::m_buffer_size_cb(jack_nframes_t nframes)
{
//	if (m_is_activated) {
//		cerr << "[JackAudioDriver] Om does not support chanding buffer size on the fly (yet).  Aborting." << endl;
//		exit(EXIT_FAILURE);
//	} else {
	m_buffer_size = nframes;
	m_jack_buffer = (jack_default_audio_sample_t*)jack_port_get_buffer(m_jack_port, m_buffer_size);
//	}
	return 0;
}













JackAudioDriver *driver;




// TUIO handler
void TuioApp::addTuioObj(unsigned int s_id, unsigned int f_id) {
	TuioObject nobj(s_id,f_id);
	objectList.push_back(nobj);

	driver->note_on = true;
// 	cout << "added " << f_id << " (" << s_id << ")" << endl;
}

void TuioApp::updateTuioObj(unsigned int s_id, unsigned int f_id, float x, float y, float a, float X, float Y, float A, float m, float r) {
	for (tuioObject = objectList.begin(); tuioObject!=objectList.end(); tuioObject++) {
		if (tuioObject->session_id==s_id) {
			tuioObject->update(x,y,a);

			a /= TWOPI; // normalise to 0..1
			driver->pitch = 440.0 * (1 - a) + 880.0 * a;
			cout << "change" << endl;
			break;
		}
	}
	cout << f_id << " (" << s_id << ") " << x << " " << y << " " << a << endl;
}

void TuioApp::removeTuioObj(unsigned int s_id, unsigned int f_id) {
	for (tuioObject = objectList.begin(); tuioObject!=objectList.end(); tuioObject++) {
		if (tuioObject->session_id==s_id) {
			objectList.erase(tuioObject);

			driver->note_on = false;
			driver->pitch = 440.0;
			cout << "remove" << endl;
			break;
		}
	}
// 	cout << "removed " << f_id << " (" << s_id << ")" << endl;
}

void TuioApp::refresh() {
}

void TuioApp::startClient(int port)
{
	client = new TuioClient(port);
	client->addTuioListener(this);
	client->start();
}


void TuioApp::stopClient()
{
	client->stop();
	delete(client);
	//running=false;
}

TuioApp::TuioApp()
{
	driver = new JackAudioDriver;
    startClient(3333);
}

TuioApp::~TuioApp()
{
	stopClient();
	delete driver;
}

