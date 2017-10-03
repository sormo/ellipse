### Ellipse
Solar system simmulator in Oxygine

N-body simmulation of solar system. Simmulation is done using boost odeint using [this](https://github.com/headmyshoulder/odeint-v2/blob/master/examples/solar_system.cpp) example.  
It is possible to move camera and zoom in and out using mouse wheel/two touches.  

Following toolbar buttons are present:  

![ellipse-toolbar](https://user-images.githubusercontent.com/7541054/31147023-c7e440e4-a888-11e7-8acf-5ecb07e61f9a.png)

#### center camera
This a toggle button. After pressing it, it is possible to click on planet/moon and camera will follow that planet/moon.  

#### simmulate system
Initially only about 115 days of system is simmulated (dt is 10'000 seconds). Using this button it is possible to simmulate additional 115 days. Simmulated data should be saved to local storage.  
 
![ellipse-jupiter-trajectory](https://user-images.githubusercontent.com/7541054/31147047-e0533f86-a888-11e7-8874-42eaf5bef275.png)  
__This picture shows initial trajectories of Jupiter and few of his moons__

#### show information
This a toggle button. After pressing it, it is possible to click on planet/moon and show weight, speed and distance from parent.  

![ellipse-moon-info](https://user-images.githubusercontent.com/7541054/31147067-efb8c3d8-a888-11e7-9e8e-bbffaf8f02f4.png)  
__This picture shows information about moon rotating around earth__  

#### create planet
This is an experimental button for creating planet. After pressing it current simmulation time is stopped (reset to time 0). By pressing screen it is possible to choose initial position and by swiping initial vector. Pressing this button again will add new planet to system and start simmulation.  

![ellipse-create](https://user-images.githubusercontent.com/7541054/31147078-fd5925dc-a888-11e7-86e8-67578c9baef8.png)  
__This picture shows process of creating body with hyperbolic trajectory around sun__  
