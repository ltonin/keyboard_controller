#include <iostream>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <cmath>
#include <limits>

#include <ros/ros.h>
#include <geometry_msgs/PointStamped.h>

// Keyboard keys definitions
#define KEY_INC_ANGLE_LEFT	'u'
#define KEY_DEC_ANGLE_LEFT	'j'
#define KEY_INC_ANGLE_RIGHT	'i'
#define KEY_DEC_ANGLE_RIGHT 'k'
#define KEY_INC_DISTANCE	'o'
#define KEY_DEC_DISTANCE	'l'
#define KEY_CMD_FORWARD		'w'
#define KEY_CMD_LEFT		'a'
#define KEY_CMD_RIGHT		'd'
#define KEY_CMD_QUIT		'q'
#define KEY_UPDATE_STEP		0.01f

enum STATES {NB_ENABLE, NB_DISABLE};

// Default angles and distance for commands
float command_angle_left	= 3*M_PI/4.0f; // -45 deg
float command_angle_right	= M_PI/4.0f;  // +45 deg
float command_angle_forward	= M_PI/2.0f; 
float command_distance		= 1.0f;

void update_parameter(float* var, float step) {
	*var = *var + step;
}

float rad2deg(float angle) {
	return angle*180.0f/M_PI;
}

float angle2frame(float angle) {
	return angle - M_PI/2.0f;
}

void dump_parameters(void) {
	printf("\tLeft at: %4.2f [deg]\tRight at: %4.2f [deg]\tDistance at: %5.2f [m]\n",
			rad2deg(angle2frame(command_angle_left)), rad2deg(angle2frame(command_angle_right)), command_distance);
}

void set_message(geometry_msgs::PointStamped& message, float angle, float distance) {

	message.header.stamp = ros::Time::now();
	message.point.x = distance*sin(angle);
	message.point.y = -distance*cos(angle);
	message.point.z = 0.0f;
}

void info(void) {
	printf("Reading from keyboard and publishing a PointStamped\n");
	printf("---------------------------------------------------\n");
	printf("Commands:\n");
	printf("\t\tw\t\t\n");
	printf("\ta\t\td\t\n");

	printf("\n\n");
	printf("u/j : increase/decrease angle for left command\n");
	printf("i/k : increase/decrease angle for right command\n");
	printf("o/l : increase/decrease distance of any command\n");
	printf("\n\n");
	printf("CTRL-C or 'q' to quit");
	printf("\n\n");
	printf("Currently:\n");
	dump_parameters();
}

int kbhit()
{
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}

void nonblock(int state) {
    struct termios ttystate;
 
    //get the terminal state
    tcgetattr(STDIN_FILENO, &ttystate);
 
    if (state==NB_ENABLE)
    {
        //turn off canonical mode
        ttystate.c_lflag &= ~ICANON;
        ttystate.c_lflag &= ~ECHO;
        //minimum of number input read.
        ttystate.c_cc[VMIN] = 1;
    }
    else if (state==NB_DISABLE)
    {
        //turn on canonical mode
        ttystate.c_lflag |= ICANON;
        ttystate.c_lflag |= ECHO;
    }
    //set the terminal attributes.
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
}


int main(int argc, char** argv){
    char key;
    int i =0;
    float angle;
    bool iscommand = false;
	bool isquit    = false;
   
	std::string		frame_id;
	std::string		topic		= "/bci_command";

	ros::init(argc, argv, "keyboard_attractor");
	ros::NodeHandle node;
	ros::Publisher	pub;
	ros::Rate r(10);

	node.param<std::string>("frame_id", frame_id, "hokuyo_link");

	// Initialize PointStamped message
	geometry_msgs::PointStamped	message;
	message.header.frame_id		= frame_id;
	
	
	// Advertise topic
    pub = node.advertise<geometry_msgs::PointStamped>(topic, 1000);

	info();

    nonblock(NB_ENABLE);
    while(ros::ok() & isquit == false) {

		i = kbhit();
		if (i!=0) {
    	    key=fgetc(stdin);
    	}


		switch(key) {
			case KEY_INC_ANGLE_LEFT:
				update_parameter(&command_angle_left, +KEY_UPDATE_STEP);
				dump_parameters();
				break;
			case KEY_DEC_ANGLE_LEFT:
				update_parameter(&command_angle_left, -KEY_UPDATE_STEP);
				dump_parameters();
				break;
			case KEY_INC_ANGLE_RIGHT:
				update_parameter(&command_angle_right, +KEY_UPDATE_STEP);
				dump_parameters();
				break;
			case KEY_DEC_ANGLE_RIGHT:
				update_parameter(&command_angle_right, -KEY_UPDATE_STEP);
				dump_parameters();
				break;
			case KEY_INC_DISTANCE:
				update_parameter(&command_distance, KEY_UPDATE_STEP);
				dump_parameters();
				break;
			case KEY_DEC_DISTANCE:
				update_parameter(&command_distance, -KEY_UPDATE_STEP);
				command_distance = command_distance < 0.0f ? 0.0f : command_distance;
				dump_parameters();
				break;
		    case KEY_CMD_FORWARD:
				set_message(message, command_angle_forward, command_distance);
				printf("\nCommand FORWARD at: %+06.2f [deg] and %5.2f [m]\n", 
						 rad2deg(angle2frame(command_angle_forward)), command_distance);
				iscommand = true;
				break;
		    case KEY_CMD_LEFT:
				set_message(message, command_angle_left, command_distance);
				printf("\nCommand LEFT at:    %+6.2f [deg] and %5.2f [m]\n", 
						 rad2deg(angle2frame(command_angle_left)), command_distance);

				iscommand = true;
				break;
		    case KEY_CMD_RIGHT:
				set_message(message, command_angle_right, command_distance);
				printf("\nCommand RIGHT at:   %+6.2f [deg] and %5.2f [m]\n", 
						 rad2deg(angle2frame(command_angle_right)), command_distance);
				iscommand = true;
				break;
			case KEY_CMD_QUIT:
				isquit	  = true;
				iscommand = false;
		    default:
				iscommand = false;
		}
		key = 0;

		if(iscommand == true) {

			//message.header.stamp = ros::Time::now();
			
			pub.publish(message);
			iscommand = false;
		}

		ros::spinOnce();
		r.sleep();
    }

    nonblock(NB_DISABLE);
    return 0;
}
