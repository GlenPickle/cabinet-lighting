Arduino controlled cabinet lighting with a Nextion Enhanced 3.2 inch TFT Display

## Design Description:

The nextion screen allows the user to interface with the lights. A leader Arduino Pro Micro listens for instructions from the Nextion Interface over serial, and sends these instructions to a follower Arduino Pro Micro. The Leader/Follower design is used to prevent Nextion signals being missed while calling the blocking functions to change the color and brightness of the lights.

## Screen Images:
![](images/home_screen.jpg "Home Screen")
![](images/color_screen.jpg "Color Selection")
![](images/bright_speed.jpg "Brightness and Speed")
![](images/settings.jpg "Settings Screen")
![](images/set_time.jpg "Set Time")

## Light Images:
![](images/cyan.jpg "Cyan")
![](images/all_colors.jpg "All Colors")
![](images/under_cabinet.jpg "Under Cabinet")

TODO: Add more images and GIFs of light animations
TODO: Add instructions to help others recreate this project more easily.
