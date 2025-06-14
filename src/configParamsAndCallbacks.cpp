#include "configParamsAndCallbacks.h"
#include "globals.h"
#include "setCallbacks.h"

void configParamsAndCallbacks(){
  static WiFiManagerParameter custom_html("<p style=\"color:pink;font-weight:Bold;\">This Is Custom HTML</p>"); // only custom html
  static WiFiManagerParameter custom_mqtt_server("server", "mqtt server", "", 40);
  static WiFiManagerParameter custom_mqtt_port("port", "mqtt port", "", 6);
  static WiFiManagerParameter custom_token("api_token", "api token", "", 16);
  static WiFiManagerParameter custom_tokenb("invalid token", "invalid token", "", 0); // id is invalid, cannot contain spaces
  static WiFiManagerParameter custom_ipaddress("input_ip", "input IP", "", 15,"pattern='\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}'"); // custom input attrs (ip mask)
  static WiFiManagerParameter custom_input_type("input_pwd", "input pass", "", 15,"type='password'"); // custom input attrs (ip mask)

  const char _customHtml_checkbox[] = "type=\"checkbox\""; 
  static WiFiManagerParameter custom_checkbox("my_checkbox", "CHECKBOX", "", 4, _customHtml_checkbox);// 4 es el numero maximo de caracteres que puede tener el valor del checkbox. en "" se podria poner un valor inicial
  const char *bufferStr = R"(
  <!-- INPUT CHOICE -->
  <br/>
  <p>Select Choice</p>
  <input style='display: inline-block;' type='radio' id='choice1' name='program_selection' value='1'>
  <label for='choice1'>Choice1</label><br/>
  <input style='display: inline-block;' type='radio' id='choice2' name='program_selection' value='2'>
  <label for='choice2'>Choice2</label><br/>
  <!-- INPUT SELECT -->
  <br/>
  <label for='input_select'>Label for Input Select</label>
  <select name="input_select" id="input_select" class="button">
  <option value="0">Option 1</option>
  <option value="1" selected>Option 2</option>
  <option value="2">Option 3</option>
  <option value="3">Option 4</option>
  </select>
  )";
  static WiFiManagerParameter custom_html_inputs(bufferStr); 

  // CALLBACKS
  wifiManager.setAPCallback(configModeCallback); 
  wifiManager.setWebServerCallback(bindServerCallback); 
  wifiManager.setSaveConfigCallback(saveWifiCallback); 
  wifiManager.setSaveParamsCallback(saveParamCallback); 
  //wifiManager.setPreOtaUpdateCallback(handlePreOtaUpdateCallback); 

  // add parameters here:
  wifiManager.addParameter(&custom_html); 
  wifiManager.addParameter(&custom_mqtt_server); 
  wifiManager.addParameter(&custom_mqtt_port); 
  wifiManager.addParameter(&custom_token); 
  wifiManager.addParameter(&custom_tokenb); 
  wifiManager.addParameter(&custom_ipaddress); 
  wifiManager.addParameter(&custom_input_type); 
  wifiManager.addParameter(&custom_checkbox); 
  wifiManager.addParameter(&custom_html_inputs); 

  // set default values 
  custom_html.setValue("test",4); 
  //custom_token.setValue("test",4); // le pone un valor llamado "test" a ese campo

  // set custom html menu content , inside menu item "custom", see setMenu()
  const char* menuhtml = "<form action='/custom' method='get'><button>Custom</button></form><br/>\n"; 
  wifiManager.setCustomMenuHTML(menuhtml); 

  wifiManager.setDarkMode(true);
  
  std::vector<const char *> menu = {"wifi","wifinoscan","info","param","custom","close","sep","erase","update","restart","exit"};

  wifiManager.setMenu(menu);

}