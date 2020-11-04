#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z) {
    //ROS_INFO_STREAM("Moving the bot to the ball");

    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;

    if (!client.call(srv))
        ROS_ERROR("Failed to call service ball_chaser");
}

int identify_side(int pixel_idx, int step) {
    int ret = 0;// stop
    int pos = (pixel_idx % step) / 3;
    int width = step / 3;
    if (pos < width / 3) {
        ret = 1;// left
    } else if (pos > 2 * width / 3) {
        ret = 2;// right
    } else if (pos >= width / 3 && pos <= 2 * width / 3) {
        ret = 3;// center
    }

    return ret;
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img) {

    int white_pixel = 255;
    float lin_x = 0.0;
    float ang_z = 0.0;
    int stop = img.height * img.step - 3;
    int side = 0;

    // Loop through each pixel in the image and check if there's a bright white one
    for (int i = 0; i < stop; i += 3) {
        if (img.data[i] == white_pixel && img.data[i + 1] == white_pixel && img.data[i + 2] == white_pixel) {
            side = identify_side(i, img.step);
            break;
        }
    }



    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera
    switch (side) {
        case 0:
            break;
        case 1:
            ang_z = 0.5;
            break;
        case 2:
            ang_z = -0.5;
            break;
        case 3:
            lin_x = 0.5;
    }
    drive_robot(lin_x, ang_z); // drive the bot
}

int main(int argc, char **argv) {
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}