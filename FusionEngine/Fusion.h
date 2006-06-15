#ifndef Header_Fusion_Info
#define Header_Fusion_Info

#if _MSC_VER > 1000
#pragma once
#endif

/*!
 * \mainpage
 * "Flow list" if you will:
 * -# Program opened
 * -# Initial client/server options objects created
 * -# User presented with title
 * -# User selects options menu
 *  -# User modifies controls, etc. then exits options menu
 *  -# Options objects updated
 * -# User selects start game
 *  -# User selects join
 *  -# Server receives join message
 *  -# Server sends 'start data check' message
 *  -# Client and server each start a new thread with a CL_Runnable derived class.
 *     The class creats and maintains a network-stream.
 *  - The new thread should send progress update signals to the original thread
 *  - Another thread could be started to allow the player to play a mini-game while the
 *    files download.
 *   -# The server sends a message containing the number of files it plans to check for
 *   -# Server sends a file name
 *    -# The client enters a for loop to check for each file using the number sent by 
 *       the server as a maximum count.
 *    -# The client checks if the filename sent above exists
 *     -# If it does exist, the client replies 'true'
 *      -# The server generates and sends a check-sum of the file for further confirmation
 *      -# If the check-sum matches the client replies 'true' and the server continues
 *         with the next file
 *     -# ...otherwise the client replies 'false'
 *     -# The server begins sending the binary data for the file over the stream.
 *     -# When sending completes, client checks the file with the check-sum again
 *        then requests resend ('false') or continue ('true') as neccesary
 *  -# When all files are synced, the thread ends, destroying the CL_Runnable object.
 *  -# The server sends the client an 'ID' (its player number in the players list)
 *     which the client must prepend to all subsequent messages.
 *  -# Client requests state initilisation and creates the ClientEnvironment
 *   -# Server replies with a frame, which the client processes as it would any other
 *      frame. However this frame is special, as its timestamp is 0, thus it will be
 *      perminantly stored (well, untill another timestamp-0 frame is received) as
 *      'origin'.
 *   -# Game proceeds. Frames are sent. Movement is predicted.
 * TODO: Complete flow list.
 */

#endif