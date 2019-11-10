/*------------------------------------------------------*/
// VISCA External in Puredata for controlling PTZ Cameras
/*------------------------------------------------------*/

// Puredata header file
#include "m_pd.h"
// libvisca must be built and installed
#include "visca/libvisca.h"
// libserialport must be built and installed
#include <libserialport.h>

static t_class *visca_class;

typedef struct _visca {
	t_object	x_obj;
	/*Structures needed for the VISCA library*/
	VISCAInterface_t iface;
	VISCACamera_t camera;
} t_visca;

/*-------------------------------------------*/
// Print Object usage
/*-------------------------------------------*/
void visca_print_usage_bang() {
	post("Usage: visca-cli [-d <serial port device>] command\n");
	post("  default serial port device: \n");      
	post("  for available commands see sourcecode...\n"); 
}
/*-------------------------------------------*/


/*-------------------------------------------*/
// Print Object info
/*-------------------------------------------*/
void visca_info(){
	post("This object written to control PTZ cameras using Visca protocol. \n"
		"Supports Sony D70/D30.\n"
		"megrimm 2019\n");
}
/*-------------------------------------------*/


/*-------------------------------------------*/
// Enumerate Com Devices
/*-------------------------------------------*/
static void visca_lstdevs() {
    post("[visca]:  available serial ports:");
    int i;
    struct sp_port **ports;

    enum sp_return error = sp_list_ports(&ports);
    if (error == SP_OK) {
      for (i = 0; ports[i]; i++) {
        post("[visca]:  %d\t%s", i, sp_get_port_name(ports[i]));
      }
      sp_free_port_list(ports);
    } else {
      post("No serial devices detected");
    }
    post("");
}
/*-------------------------------------------*/


/*-------------------------------------------*/
// Open Visca Interface
/*-------------------------------------------*/
void visca_opencom(t_visca *x, t_symbol *s, int argc, t_atom *argv) {
  	char comstr[1000];
  	int camera_num;
  	atom_string(argv, comstr, 1000);
	
  	if (argc<1){
		post("Please provide a serial port device. Ex. /dev/cu.usbserial-FTGBV1NE\n");
    	}
	if (argc>=1){
		//atom_string(argv, comstr, 1000);	
  		if (VISCA_open_serial(&x->iface, comstr)==VISCA_SUCCESS) {
	  	  	post(comstr);
	  		post("Serial Connection Established");
  		}
		else{
			post("Serial Connection Unsuccessful");
		}
	}

  	x->iface.broadcast=0;
  	VISCA_set_address(&x->iface, &camera_num);
  	if(VISCA_set_address(&x->iface, &camera_num)!=VISCA_SUCCESS) {
		#ifdef WIN
    	_RPTF0(_CRT_WARN,"unable to set address\n");
		#endif
    	post("visca-cli: unable to set address\n");
    	VISCA_close_serial(&x->iface);
  	}

  	x->camera.address=1;

  	if(VISCA_clear(&x->iface, &x->camera)!=VISCA_SUCCESS) {
		#ifdef WIN
    	_RPTF0(_CRT_WARN,"unable to clear interface\n");
		#endif
    	post("visca-cli: unable to clear interface\n");
    	VISCA_close_serial(&x->iface);
  	}
  	if(VISCA_get_camera_info(&x->iface, &x->camera)!=VISCA_SUCCESS) {
		#ifdef WIN
    	_RPTF0(_CRT_WARN,"unable to oget camera infos\n");
		#endif
    	post("visca-cli: unable to oget camera infos\n");
    	VISCA_close_serial(&x->iface);
  	}
  	post("Camera initialisation successful.\n");
}
/*------------------------------------------------------*/

/*-------------------------------------------*/
// Close Visca Interface
/*-------------------------------------------*/
void visca_closecom(t_visca *x){
  	// read the rest of the data: (should be empty)
  	unsigned char packet[3000];
  	uint32_t buffer_size = 3000;

  	VISCA_usleep(2000);

  	if (VISCA_unread_bytes(&x->iface, packet, &buffer_size)!=VISCA_SUCCESS){
    	uint32_t i;
    	post("ERROR: %u bytes not processed", buffer_size);
    	for (i=0;i<buffer_size;i++)
      	  	post("%2x ",packet[i]);
    		post("\n");
  	}
  	if(VISCA_close_serial(&x->iface)==VISCA_SUCCESS){
	post("Connection Closed");
	}
}
/*-------------------------------------------*/

/*------------------------------------------------------*/
// TEST Visca
/*------------------------------------------------------*/
void visca_testcom(t_visca *x, t_symbol *s, t_int argc, t_atom *argv) {
	char comstr[1000];
	atom_string(argv, comstr, 1000);
	
  	int camera_num;
  	uint8_t value;
  	uint16_t zoom;
  	int pan_pos, tilt_pos;
	
  	if (argc<1){
		post("Please provide a serial port device. Ex. /dev/cu.usbserial-FTGBV1NE\n");
    	}
	if (VISCA_open_serial(&x->iface, comstr)!=VISCA_SUCCESS){
		        post("%s: unable to open serial device %s\n",argv[0],argv[1]);
		      }
	x->iface.broadcast=0;
  	VISCA_set_address(&x->iface, &camera_num);
  	x->camera.address=1;
  	VISCA_clear(&x->iface, &x->camera);

  	VISCA_get_camera_info(&x->iface, &x->camera);
  	post("Some camera info:\n------------------\n");
  	post("vendor: 0x%04x\n model: 0x%04x\n ROM version: 0x%04x\n socket number: 0x%02x\n",
		x->camera.vendor, x->camera.model, x->camera.rom_version, x->camera.socket_num);
		
	VISCA_usleep(500000);

  	if (VISCA_set_zoom_value(&x->iface, &x->camera, 0x0000)!=VISCA_SUCCESS)
    	post("error setting zoom\n");

  	VISCA_usleep(500000);

  	if (VISCA_set_zoom_value(&x->iface, &x->camera, 0x4000)!=VISCA_SUCCESS)
    	post("error setting zoom\n");

  	VISCA_usleep(500000);

  	if (VISCA_set_zoom_value(&x->iface, &x->camera, 0x1234)!=VISCA_SUCCESS)
    	post("error setting zoom\n");
  	if (VISCA_get_zoom_value(&x->iface, &x->camera, &zoom)!=VISCA_SUCCESS)
    	post("error getting zoom\n");
  	else
    	post("Zoom value: 0x%04x",zoom);

  	VISCA_usleep(500000);

  	if (VISCA_set_zoom_value(&x->iface, &x->camera, 0x0000)!=VISCA_SUCCESS)
    	post("error setting zoom");
  	if (VISCA_get_power(&x->iface, &x->camera, &value)!=VISCA_SUCCESS)
    	post("error getting power\n");
  	else
    	post("power status: 0x%02x",value);
  	if (VISCA_set_pantilt_reset(&x->iface, &x->camera)!=VISCA_SUCCESS)
    	post("error setting pan tilt home\n");
  	else
   		post("Setting pan tilt home");
  	if (VISCA_set_pantilt_absolute_position(&x->iface, &x->camera,5,5,-500,-200)!=VISCA_SUCCESS)
    	post("error setting pan tilt absolute position with negative position\n");
  	else
    	post("Setting pan tilt absolute position");
  	if (VISCA_get_pantilt_position(&x->iface, &x->camera, &pan_pos, &tilt_pos)!=VISCA_SUCCESS)
    	post("error getting pan tilt absolute position\n");
  	else
    	post("Absolute position, Pan value: %d, Tilt value: %d",pan_pos,tilt_pos);
  	if (VISCA_set_pantilt_absolute_position(&x->iface, &x->camera,18,14,500,200)!=VISCA_SUCCESS)
    	post("error setting pan tilt absolute position with positive position\n");
  	else
    	post("Setting pan tilt absolute position");
  	if (VISCA_get_pantilt_position(&x->iface, &x->camera, &pan_pos, &tilt_pos)!=VISCA_SUCCESS)
    	post("error getting pan tilt absolute position\n");
  	else
    	post("Absolute position, Pan value: %d, Tilt value: %d",pan_pos,tilt_pos);
  	if (VISCA_set_pantilt_home(&x->iface, &x->camera)!=VISCA_SUCCESS)
    	post("error setting pan tilt home\n");
  	else
    	post("Setting pan tilt home\n");

  	VISCA_set_zoom_value(&x->iface, &x->camera, 0x0D00);
  	VISCA_set_shutter_value(&x->iface, &x->camera, 0x0D00);{
    	unsigned char packet[3000];
    	uint32_t buffer_size = 3000;
    	if (VISCA_unread_bytes(&x->iface, packet, &buffer_size)!=VISCA_SUCCESS){
      	  uint32_t i;
		  post( "ERROR: %u bytes not processed", buffer_size);
      	for (i=0;i<buffer_size;i++)
        	post("%2x ",packet[i]);
      		post("\n");
    	}
  	}
  	VISCA_close_serial(&x->iface);
}

/*-----------------------------------------------------*/

/*------------------------------------------------------*/
// TESTS (Other)
/*------------------------------------------------------*/
// Sends [passfloat $1{ float straight out first outlet
void visca_passfloat(t_visca *x, t_floatarg f){
	outlet_float(x->x_obj.ob_outlet, f);
}
// Sends [addone $1{ adds one to incoming number
void visca_addone(t_visca *x, t_floatarg f){
	int a = f + 1;
	outlet_float(x->x_obj.ob_outlet, a);
}
// Testing Passing Symbols
void visca_testsym(t_visca *x, t_symbol *s, t_int argc, t_atom *argv){
	if( argc < 1 ){
	        post("no arguments", argv, s, x );
	    }
	if( argc == 1 ){
			char teststring[1000];
			atom_string(argv, teststring, 1000);
			post("one argument = ");
			post(teststring);
		}
	if( argc >= 2 ){
			post("multiple arguments");
		}
}

void visca_pantest(t_visca *x){
	int pan_pos, tilt_pos;
	
  	if (VISCA_set_pantilt_absolute_position(&x->iface, &x->camera,5,5,-500,-200)!=VISCA_SUCCESS)
    	post("error setting pan tilt absolute position with negative position\n");
  	else
    	post("Setting pan tilt absolute position");
  	if (VISCA_get_pantilt_position(&x->iface, &x->camera, &pan_pos, &tilt_pos)!=VISCA_SUCCESS)
    	post("error getting pan tilt absolute position\n");
  	else
    	post("Absolute position, Pan value: %d, Tilt value: %d",pan_pos,tilt_pos);
  	if (VISCA_set_pantilt_absolute_position(&x->iface, &x->camera,18,14,500,200)!=VISCA_SUCCESS)
    	post("error setting pan tilt absolute position with positive position\n");
  	else
    	post("Setting pan tilt absolute position");
  	if (VISCA_get_pantilt_position(&x->iface, &x->camera, &pan_pos, &tilt_pos)!=VISCA_SUCCESS)
    	post("error getting pan tilt absolute position\n");
  	else
    	post("Absolute position, Pan value: %d, Tilt value: %d",pan_pos,tilt_pos);
  	if (VISCA_set_pantilt_home(&x->iface, &x->camera)!=VISCA_SUCCESS)
    	post("error setting pan tilt home\n");
  	else
    	post("Setting pan tilt home\n");
}
/*-----------------------------------------------------*/


void *visca_new(){
	t_visca *x = (t_visca *)pd_new(visca_class);
	outlet_new(&x->x_obj, &s_float);	
	return (void *) x;
}

//void visca_free(t_visca *x){
    //post("[visca] free serial...");
    //clock_unset(x->x_clock);
    //clock_free(x->x_clock);
    //x->comhandle = visca_close(x);
    //freebytes(x->x_inbuf, x->x_inbuf_len);
    //freebytes(x->x_outbuf, x->x_outbuf_len);
//}


void visca_setup(void){
	visca_class = 
		class_new(gensym("visca"),(t_newmethod)visca_new,0,sizeof(t_visca),CLASS_DEFAULT,A_GIMME,0);
		//class_new(gensym("visca"),(t_newmethod)visca_new,(t_method)visca_free,sizeof(t_visca),CLASS_DEFAULT,A_GIMME,0);
		// Function "visca_print_usage_bang() called whenever a [bang] is recieved
		class_addbang(visca_class,visca_print_usage_bang);
		// Function "visca_info()" called whenever a [info{ message is sent to the first inlet
		class_addmethod(visca_class,(t_method)visca_info,gensym("info"),0);
		// Function "visca_testfloat()" called with [passfloat $1{ message sendint float to outlet
		class_addmethod(visca_class,(t_method)visca_passfloat,gensym("passfloat"),A_DEFFLOAT,0);
		// Function "visca_addone()" called with [addone $1{ message adds one to float
		class_addmethod(visca_class,(t_method)visca_addone,gensym("addone"),A_DEFFLOAT,0);
		// Send a Command
		class_addmethod(visca_class, (t_method)visca_testsym, gensym("testsymbol"),A_GIMME, 0);
		// Function to list Com Devices
		class_addmethod(visca_class, (t_method)visca_lstdevs, gensym("devices"), 0);
		// Send a Command
		class_addmethod(visca_class, (t_method)visca_testcom, gensym("test"),A_GIMME, 0);
		// Open Interface
		class_addmethod(visca_class, (t_method)visca_opencom, gensym("open"),A_GIMME, 0);
		// Close Interface
		class_addmethod(visca_class, (t_method)visca_closecom, gensym("close"), 0);
		// Test Paning After Connection Open
		class_addmethod(visca_class, (t_method)visca_pantest, gensym("pan"), 0);
		
	    verbose(-1, "-----------------------------------\n"
					"visca - PD external for unix/windows\n"
	        		"controls a PTZ camera using visca protocol\n"
	        		"megrimm 2019\n"
					"-----------------------------------\n");
}