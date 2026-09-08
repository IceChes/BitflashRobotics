//controller vars
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
ControllerPtr myControllers[BP32_MAX_GAMEPADS];


//FUNCTION ZONE
void triggerFailsafe() {
  //Failsafe by setting all the controller variables to their rest states.
  //The rate limit code for the weapon will handle the throttling-down.
  Serial.println("ERROR: Controller disconnected! Failsafing...");
  ls = 0;
  rs = 0;
  lb = false;
  rb = false;
  lt = 0;
  rt = 0;
  a = false;
  b = false;
  x = false;
  y = false;
  buttons_state = 0;
  dpad_state = 0;
  super = false;
}

//Bluepad32 connection callback. I don't know how this works, but I am going to add some failsafe code in there.
void onConnectedController(ControllerPtr ctl) {
  Serial.println("BLUEPAD: Controller is connected!");
  myControllers[0] = ctl;
}

//ctl->playDualRumble(0 /* delayedStartMs */, 500 /* durationMs */, 127 /* weakMagnitude */, 127 /* strongMagnitude */);

//Bluepad32 disconnect callback
void onDisconnectedController(ControllerPtr ctl) {
  Serial.println("ERROR: Lost controller connection! Failsafing...");
  BP32.enableNewBluetoothConnections(true);
  triggerFailsafe();
}

void processGamepad(ControllerPtr ctl) {

  ls = ctl->axisY();
  rs = ctl->axisRY();
  lb = ctl->l1();
  rb = ctl->r1();
  lt = ctl->brake();
  rt = ctl->throttle();
  a = ctl->a();
  b = ctl->b();
  x = ctl->x();
  y = ctl->y();
  buttons_state = ctl->buttons();
  dpad_state = ctl->dpad();
  super = ctl->miscSystem();
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



//not used yet, may never be used, will remove if that's the case
struct FireflyStatus {
  bool driver_ok;
  bool acc_ok;
  bool firefly_ok;
};


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
      if (USER_VERBOSE_LOGGING) {
        Serial.println("INFO: Impulse online.");  //isn't it so nice to say that something works?
      }                                           //impulse isn't able to say anything other than that it booted, so we don't even have to check the payload
      break;
  }
}

//input data handling system
//The following 
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

  if (_d.device >= 0 && _d.device <= registry_size) {     //making sure the device's address is inside the registry
    if (device_registry[_d.device].process != nullptr) {  //making sure the device actually has a processing function
      device_registry[_d.device].process(_d);             //do the thing
    } else {
      if (USER_VERBOSE_LOGGING) {
        Serial.println("INFO: Device ID " + String(_d.device) + " sent data to host, but it does not have a data processor. Ignoring.");
      }
    }
  } else {
    Serial.println("ERROR: Got data from device ID " + String(_d.device) + " which does not exist in the device registry.");
  }
}