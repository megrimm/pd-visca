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
static void visca_devices()
{
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


/*------------------------------------------------------*/
// TESTS
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
/*-----------------------------------------------------*/


void *visca_new(){
	
	t_visca *x = (t_visca *)pd_new(visca_class);
	
	outlet_new(&x->x_obj, &s_float);
	
	return (void *) x;
}

void visca_setup(void){
	visca_class = 
		class_new(gensym("visca"),(t_newmethod)visca_new,0,sizeof(t_visca),CLASS_DEFAULT,A_GIMME,0);
		// Function "visca_print_usage_bang() called whenever a [bang] is recieved
		class_addbang(visca_class,visca_print_usage_bang);
		// Function "visca_info()" called whenever a [info{ message is sent to the first inlet
		class_addmethod(visca_class,(t_method)visca_info,gensym("info"),0);
		// Function "visca_testfloat()" called with [passfloat $1{ message sendint float to outlet
		class_addmethod(visca_class,(t_method)visca_passfloat,gensym("passfloat"),A_DEFFLOAT,0);
		// Function "visca_addone()" called with [addone $1{ message adds one to float
		class_addmethod(visca_class,(t_method)visca_addone,gensym("addone"),A_DEFFLOAT,0);
		// Function to list Com Devices
		class_addmethod(visca_class, (t_method)visca_devices, gensym("devices"), 0);
		
	    verbose(-1, "-----------------------------------\n"
					"visca - PD external for unix/windows\n"
	        		"controls a PTZ camera using visca protocol\n"
	        		"megrimm 2019\n"
					"-----------------------------------\n");
}