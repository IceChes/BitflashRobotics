//current computer state
struct Firefly {

  bool driver_alive;
  bool acc_alive;

  bool firefly_ok;

  unsigned long main_timer;
  unsigned long last_driver_heartbeat;
  unsigned long last_acc_heartbeat;
  unsigned long last_button_press;
  unsigned long last_esc_update_time;
  unsigned long last_debug_update;

  int esc_value = 1000;
  int target_esc_value;
  int motor_1_value;
  int motor_2_value;
  bool motor_1_active;
  bool motor_2_active;
  bool inverted = false;
  bool weapon_locked = false;
};

//controller vars
struct {
  int ls;      //left stick
  int rs;      //right stick
  int inv_ls;  //the stick value after the inversion logic processes it
  int inv_rs;
  int used_ls;  //the stick value the code will actually use
  int used_rs;
  bool lb;  //left bumper
  bool rb;  //right bumper
  int lt;   //left trigger
  int rt;   //right trigger
  bool a;
  bool b;
  bool x;
  bool y;
  int dpad_state;  //1=up, 2=down, 4=right, 8=left
  int buttons_state;
  bool super;         //xbox/stadia button
  bool data_updated;  //if there is new controller data
  bool controller_connected = false;
} stadia;

ControllerPtr myControllers[BP32_MAX_GAMEPADS];

//FUNCTION ZONE
void triggerFailsafe() {
  //Failsafe by setting all the controller variables to their rest states.
  //The rate limit code for the weapon will handle the throttling-down.
  Serial.println("ERROR: Controller disconnected! Failsafing...");
  stadia.ls = 0;
  stadia.rs = 0;
  stadia.lb = false;
  stadia.rb = false;
  stadia.lt = 0;
  stadia.rt = 0;
  stadia.a = false;
  stadia.b = false;
  stadia.x = false;
  stadia.y = false;
  stadia.buttons_state = 0;
  stadia.dpad_state = 0;
  stadia.super = false;
}

//Bluepad32 connection callback. I don't know how this works, but I am going to add some failsafe code in there.
void onConnectedController(ControllerPtr ctl) {
  Serial.println("BLUEPAD: Controller is connected!");
  stadia.controller_connected = true;
  myControllers[0] = ctl;
}

//ctl->playDualRumble(0 /* delayedStartMs */, 500 /* durationMs */, 127 /* weakMagnitude */, 127 /* strongMagnitude */);

//Bluepad32 disconnect callback
void onDisconnectedController(ControllerPtr ctl) {
  Serial.println("ERROR: Lost controller connection! Failsafing...");
  stadia.controller_connected = false;
  BP32.enableNewBluetoothConnections(true);
  triggerFailsafe();
}

void processGamepad(ControllerPtr ctl) {

  stadia.ls = ctl->axisY();
  stadia.rs = ctl->axisRY();
  stadia.lb = ctl->l1();
  stadia.rb = ctl->r1();
  stadia.lt = ctl->brake();
  stadia.rt = ctl->throttle();
  stadia.a = ctl->a();
  stadia.b = ctl->b();
  stadia.x = ctl->x();
  stadia.y = ctl->y();
  stadia.buttons_state = ctl->buttons();
  stadia.dpad_state = ctl->dpad();
  stadia.super = ctl->miscSystem();
  //This was in the example code. Saving it for later:
  //ctl->playDualRumble(0 /* delayedStartMs */, 2000 /* durationMs */, 255 /* weakMagnitude */, 255 /* strongMagnitude */);
}

void rumble(int _dur, int _pwr_weak, int _pwr_strng, ControllerPtr ctl) {
  ctl->playDualRumble(0, _dur, _pwr_weak, _pwr_strng);
}

//I have no idea what this does. This is a great library, if only they would document it.
void processControllers() {
  for (auto myController : myControllers) {
    if (myController && myController->isConnected() && myController->hasData()) {
      processGamepad(myController);
    }
  }
}







//the spooky zone of inbound data handling. all this stuff actually uses the packets that are built by the above function
//impulse's handling function, others will be added here
void processImpulseData(PacketData &_d) {
  switch (_d.command) {
    case IMPULSE_OUTBOUND_ERROR:
      if (USER_VERBOSE_LOGGING) {
        Serial.print("WARN: Impulse error ");
        Serial.println(_d.payload);
      }
      break;
    case IMPULSE_OUTBOUND_INFO:
      switch (_d.payload) {
        case GLOBAL_INFO_DEVICE_READY:
          if (USER_VERBOSE_LOGGING) {
            Serial.println("INFO: Impulse online");
          }
          break;
        case GLOBAL_INFO_HEARTBEAT:
          if (USER_VERBOSE_LOGGING) {
            Serial.println("INFO: Got heartbeat from Impulse");
          }
          break;
      }
      break;
  }
}

//input data handling system
typedef void (*DataProcessor)(PacketData &);

//a single entry in the device registry looks like this
struct DeviceRegistry {
  const char *name;
  const byte id;
  DataProcessor process;
};

//master device registry
//human-readable name, bfr addr, and cooresponding data handling function
const DeviceRegistry device_registry[]{
  //firefly doesn't get a data handling function because it's the host device. it should never be recieving data from itself.
  { "Firefly", 0, nullptr },
  { "Impulse", 1, processImpulseData },
};

//the thing that does stuff :thumbsup:
void handleInboundData(PacketData &_d) {

  int registry_size = sizeof(device_registry) / sizeof(DeviceRegistry);  //divide the total size of the device registry by the size of a single entry to get the number of items in the registry

  if (_d.device >= 0 && _d.device < registry_size) {      //making sure the device's address is inside the registry
    if (device_registry[_d.device].process != nullptr) {  //making sure the device actually has a processing function
      device_registry[_d.device].process(_d);             //do the thing
    } else {
      if (USER_VERBOSE_LOGGING) {
        Serial.println("INFO: Device ID " + String(_d.device) + " sent data to host, but it does not have a data processor. Ignoring.");
      }
    }
  } else {
    Serial.println("ERROR: Got data from device ID " + String(_d.device) + ", which does not exist in the device registry.");
  }
}