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
	t_object x_obj;
	t_outlet *bang_out;
	t_outlet *float_out;
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
// Enumerate Com Devices (uses libserialport)
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
// Process Commands
/*------------------------------------------------------*/
char *process_command(int argc, char **argv) {
  /*loop counter*/
  int i;
  /*temporarily used to hold the length of a string*/
  size_t length = 0;
  /*used to hold the command that is returned*/
  char *command;
  /*at least a command has to be specified*/
  if (argc < 2) {
 //   print_usage();
	  visca_print_usage_bang();
  }
  /*find total length of command*/
  length = 0;
  for (i=1; i < argc; i++) {
    length += strlen(argv[i])+1;
  }
  /*allocate memory for command*/
  command = (char *)malloc(sizeof(char) * length);
  /*copy the first argument to the commandline*/
  strcpy(command, argv[1]);
  /*add the rest of the arguments, seperated by blanks*/
  for (i=2; i < argc; i++) {
    strcat(command, " ");
    strcat(command, argv[i]);
  }
  return command;  
}
/*------------------------------------------------------*/

/*------------------------------------------------------*/
// Send Commands
/*------------------------------------------------------*/
int visca_send_command(int argc, char **argv) {
  char *command;
  int errorcode, ret1, ret2, ret3;

  command = process_command(argc, argv);

  errorcode = doCommand(command, &ret1, &ret2, &ret3);
  switch(errorcode) {
    case 10:
      printf("10 OK - no return value\n");
      break;
    case 11:
      printf("11 OK - one return value\nRET1: %i\n", ret1);
      break;    
    case 12:
      printf("12 OK - two return values\nRET1: %i\nRET2: %i\n", ret1, ret2);
      break;
    case 13:
      printf("13 OK - three return values\nRET1: %i\nRET2: %i\nRET3: %i\n", 
             ret1, ret2, ret3);
      break;
    case 40:
      printf("40 ERROR - command not recognized\n");
      break;
    case 41:
      printf("41 ERROR - argument 1 not recognized\n");
      break;
    case 42:
      printf("42 ERROR - argument 2 not recognized\n");
      break;
    case 43:
      printf("43 ERROR - argument 3 not recognized\n");
      break;
    case 44:
      printf("44 ERROR - argument 4 not recognized\n");
      break;
    case 45:
      printf("45 ERROR - argument 5 not recognized\n");
      break;
    case 46:
      printf("46 ERROR - camera replied with an error\n");
      break;
    case 47:
      printf("47 ERROR - camera replied with an unknown return value\n");
      break;
    default:
      printf("unknown error code: %i\n", errorcode);
  }
}
/*------------------------------------------------------*/

/*------------------------------------------------------*/
// Commands
/*------------------------------------------------------*/
int doCommand(t_visca *x, char *commandline, int *ret1, int *ret2, int *ret3) {
  /*Variables for the user specified command and arguments*/
  char *command;
  char *arg1;
  char *arg2;
  char *arg3;
  char *arg4;
  char *arg5;
  int intarg1=0;
  int intarg2=0;
  int intarg3=0;
  int intarg4=0;
  int intarg5=0;
  int boolarg;
  VISCATitleData_t *temptitle;
  
  /*Variables that hold return values from VISCA routines*/
  uint8_t value8, value8b, value8c;
  uint16_t value16;
  
  /*tokenize the commandline*/
  command = strtok(commandline, " ");
  arg1 = strtok(NULL, " ");
  arg2 = strtok(NULL, " ");
  arg3 = strtok(NULL, " ");
  arg4 = strtok(NULL, " ");
  arg5 = strtok(NULL, " ");
  
  /*Try to convert the arguments to integers*/
  if (arg1 != NULL) {
    intarg1 = atoi(arg1);
  }
  if (arg2 != NULL) {
    intarg2 = atoi(arg2);
  }
  if (arg3 != NULL) {
    intarg3 = atoi(arg3);
  }
  if (arg4 != NULL) {
    intarg4 = atoi(arg4);
  }
  if (arg5 != NULL) {
    intarg5 = atoi(arg5);
  }
  
  /*Try to find a boolean value*/
  if ((arg1 != NULL) && (strcmp(arg1, "true") == 0)) {
    boolarg = 2;
  } else if ((arg1 != NULL) && (strcmp(arg1, "false") == 0)) {
    boolarg = 3;
  } else if ((arg1 != NULL) && (strcmp(arg1, "1") == 0)) {
    boolarg = 2;
  } else if ((arg1 != NULL) && (strcmp(arg1, "0") == 0)) {
    boolarg = 3;
  } else {
    boolarg = -1;
  }
  
#if DEBUG
  fprintf(stderr, "command: %s\n", command);
  fprintf(stderr, "arg1: %s\narg2: %s\narg3: %s\narg4: %s\narg5: %s\n", 
          arg1, arg2, arg3, arg4, arg5);
  fprintf(stderr, 
          "intarg1: %i\nintarg2: %i\nintarg3: %i\nintarg4:%i\nintarg5:%i\n", 
          intarg1, intarg2, intarg3, intarg4, intarg5);
  fprintf(stderr, "boolarg: %i\n", boolarg);
#endif

// TEST COMMANDS
  if (strcmp(command, "set_zoom_tele") == 0) {
    if (VISCA_set_zoom_tele(&x->iface, &x->camera)!=VISCA_SUCCESS) {
      return 46;
    }
    return 10;
  }
  if (strcmp(command, "set_pantilt_left") == 0) {
    if ((arg1 == NULL) || (intarg1 < 1) || (intarg1 > 24)) {
      return 41;
    }
    if ((arg2 == NULL) || (intarg2 < 1) || (intarg2 > 20)) {
      return 42;
    }
    if (VISCA_set_pantilt_left(&x->iface, &x->camera, intarg1, intarg2)
        != VISCA_SUCCESS) {
      return 46;
    }
    return 10;
  }

  if (strcmp(command, "set_pantilt_right") == 0) {
    if ((arg1 == NULL) || (intarg1 < 1) || (intarg1 > 24)) {
      return 41;
    }
    if ((arg2 == NULL) || (intarg2 < 1) || (intarg2 > 20)) {
      return 42;
    }
    if (VISCA_set_pantilt_right(&x->iface, &x->camera, intarg1, intarg2)
        != VISCA_SUCCESS) {
      return 46;
    }
    return 10;
  }
// END TEST COMMANDS  
}
/*------------------------------------------------------*/

/*------------------------------------------------------*/
// TESTS (Other)
/*------------------------------------------------------*/
// Sends [passfloat $1{ float straight out first outlet
void visca_passfloat(t_visca *x, t_floatarg f){
	//outlet_float(x->x_obj.ob_outlet, f);
	outlet_float(x->float_out, f);
}
// Sends [addone $1{ adds one to incoming number
void visca_addone(t_visca *x, t_floatarg f){
	int a = f + 1;
	outlet_float(x->float_out, a);
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
// Testing Pan/Tilt After Open
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
	outlet_bang(x->bang_out);
}
/*-----------------------------------------------------*/


void *visca_new(){
	t_visca *x = (t_visca *)pd_new(visca_class);
	x->float_out = outlet_new(&x->x_obj, &s_float);
	x->bang_out = outlet_new(&x->x_obj, &s_bang);	
	return (void *) x;
}

void visca_free(t_visca *x){
	outlet_free(x->bang_out);
	outlet_free(x->float_out);
}


void visca_setup(void){
	visca_class = 
		//class_new(gensym("visca"),(t_newmethod)visca_new,0,sizeof(t_visca),CLASS_DEFAULT,A_GIMME,0);
		class_new(gensym("visca"),(t_newmethod)visca_new,(t_method)visca_free,sizeof(t_visca),CLASS_DEFAULT,A_GIMME,0);
		// Function "visca_print_usage_bang() called whenever a [bang] is recieved
		class_addbang(visca_class,visca_print_usage_bang);
		// Function "visca_info()" called whenever a [info{ message is sent to the first inlet
		class_addmethod(visca_class,(t_method)visca_info,gensym("info"),0);
		// Function "visca_testfloat()" called with [passfloat $1{ message sendint float to outlet
		class_addmethod(visca_class,(t_method)visca_passfloat,gensym("passfloat"),A_DEFFLOAT,0);
		// Function "visca_addone()" called with [addone $1{ message adds one to float
		class_addmethod(visca_class,(t_method)visca_addone,gensym("addone"),A_DEFFLOAT,0);
		// Test for the Passing of Symbols
		class_addmethod(visca_class, (t_method)visca_testsym, gensym("testsymbol"),A_GIMME, 0);
		// Function to list Com Devices
		class_addmethod(visca_class, (t_method)visca_lstdevs, gensym("devices"), 0);
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