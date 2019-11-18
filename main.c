#include <wiiuse.h>
#include <stdio.h>
#include <raylib.h>
#include <unistd.h>
#include <math.h>

#define MAX_WIIMOTES 1

bool keepWindow = true;
Vector3 spherePos;
Vector3 sphereVelocity;
Vector3 sphereAcceleration;

void handle_event(struct wiimote_t* wm)
{
    if (WIIUSE_USING_ACC(wm))
    {
        // sphereAcceleration = (Vector3) { wm->accel.x - average.x, wm->accel.y - average.y, wm->accel.z - average.z };
        sphereAcceleration = (Vector3) { wm->gforce.x, -wm->gforce.z + 1, -wm->gforce.y };
        printf("%d, %d, %d : ", wm->gforce.x, wm->gforce.y, wm->gforce.z);
    }
}

void handle_ctrl_status(struct wiimote_t* wm)
{
	printf("\n\n--- CONTROLLER STATUS [wiimote id %i] ---\n", wm->unid);

	printf("attachment:      %i\n", wm->exp.type);
	printf("speaker:         %i\n", WIIUSE_USING_SPEAKER(wm));
	printf("ir:              %i\n", WIIUSE_USING_IR(wm));
	printf("leds:            %i %i %i %i\n", WIIUSE_IS_LED_SET(wm, 1), WIIUSE_IS_LED_SET(wm, 2), WIIUSE_IS_LED_SET(wm, 3), WIIUSE_IS_LED_SET(wm, 4));
	printf("battery:         %f %%\n", wm->battery_level);
}

void handle_disconnect(wiimote* wm)
{
    printf("\n\n--- DISCONNECTED [wiimote id %i] ---\n", wm->unid);
    keepWindow = false;
}

//check if a wiimote is connected
short any_wiimote_connected(wiimote** wm, int wiimotes)
{
	int i;
	if (!wm) {
		return 0;
	}

	for (i = 0; i < wiimotes; i++) {
		if (wm[i] && WIIMOTE_IS_CONNECTED(wm[i])) {
			return 1;
		}
	}

	return 0;
}

// Vector3 TruncateFloat(Vector3 in)
// {
//     in.x = (int)(in.x * 100);
//     in.y = (int)(in.y * 100);
//     in.z = (int)(in.z * 100);
//     in.x = (float)(in.x / 100);
//     in.y = (float)(in.y / 100);
//     in.z = (float)(in.z / 100);

//     if (in.x > 100 || in.x < -100) in.x = 0;
//     if (in.y > 100 || in.y < -100) in.y = 0;
//     if (in.z > 100 || in.z < -100) in.z = 0;

//     return in;
// }

int main()
{
    // raylib varibles
    const int screenWidth = 600;
    const int screenHeight = 600;

    // wiiuse varibles
    wiimote** wiimotes;
    int found, connected;
    // xScale = (float)screenWidth / 1024.0f;
    // yScale = (float)screenHeight / 768.0f;

    // Init wiiuse
    wiimotes = wiiuse_init(MAX_WIIMOTES);

    // find wiimotes
    printf("[INFO] Looking for Wiimotes!\n");
    found = wiiuse_find(wiimotes, MAX_WIIMOTES, 5);
    if (!found)
    {
        printf("[ERROR] Failed to find any wiimotes!\n");
        return 0;
    }

    // Connect to wiimotes
    connected = wiiuse_connect(wiimotes, MAX_WIIMOTES);
    if (connected)
    {
        printf("[INFO] Connected to %i wiimotes of %i found!\n", connected, found);
    } else 
    {
        printf("[ERROR] Failed to connect to any wiimotes.\n");
        return 0;
    }

    wiiuse_set_leds(wiimotes[0], WIIMOTE_LED_1);
    wiiuse_rumble(wiimotes[0], 1);
    usleep(500000);
    wiiuse_rumble(wiimotes[0], 0);
    wiiuse_motion_sensing(wiimotes[0], 1);

    // init raylib
    InitWindow(screenWidth, screenHeight, "Wiimote spacial tracking");

    Camera camera = { 0 };
    camera.position = (Vector3) { 1.0f, 2.0f, 1.0f };
    camera.target = (Vector3) { 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3) { 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.type = CAMERA_PERSPECTIVE;

    SetCameraMode(camera, CAMERA_FIRST_PERSON);

    SetTargetFPS(120);
    while(!WindowShouldClose() && any_wiimote_connected(wiimotes, MAX_WIIMOTES) && keepWindow)
    {
        // Update

        UpdateCamera(&camera);

        if (wiiuse_poll(wiimotes, MAX_WIIMOTES))
        {
            for (int i = 0; i < MAX_WIIMOTES; i++)
            {
                switch (wiimotes[i]->event)
                {
                    case WIIUSE_EVENT:
                    {
                        handle_event(wiimotes[i]);
                    } break;

                    case WIIUSE_STATUS:
                    {
                        handle_ctrl_status(wiimotes[i]);
                    } break;

                    case WIIUSE_DISCONNECT:
                    case WIIUSE_UNEXPECTED_DISCONNECT:
                    {
                        handle_disconnect(wiimotes[i]);
                    } break;
                    default: break;
                }
            }
        }

        sphereAcceleration = (Vector3)
        {
            (sphereAcceleration.x * 9.8f) * GetFrameTime(),
            (sphereAcceleration.y * 9.8f) * GetFrameTime(),
            (sphereAcceleration.z * 9.8f) * GetFrameTime()
        };
        

        // sphereAcceleration = (Vector3)
        // {
        //     roundf(sphereAcceleration.x * 9.8f),
        //     roundf(sphereAcceleration.y * 9.8f),
        //     roundf(sphereAcceleration.z * 9.8f)
        // };

        printf("%0.1f, %0.1f, %0.1f\n", sphereAcceleration.x, sphereAcceleration.y, sphereAcceleration.z);
        sphereVelocity = (Vector3)
        {
            (sphereAcceleration.x) + sphereVelocity.x,
            (sphereAcceleration.y) + sphereVelocity.y,
            (sphereAcceleration.z) + sphereVelocity.z
        }; 

        spherePos = (Vector3) 
        { 
            sphereVelocity.x + spherePos.x,
            sphereVelocity.y + spherePos.y,
            sphereVelocity.z + spherePos.z
        };

        // Draw
        BeginDrawing();
            ClearBackground(BLACK);
            BeginMode3D(camera);
                //DrawSphere(spherePos, 0.1f, GREEN);
                DrawSphere(sphereAcceleration, 0.1f, GRAY);
                DrawSphere(sphereVelocity, 0.1f, LIGHTGRAY);
                DrawSphere(spherePos, 0.1f, WHITE);
                DrawSphere((Vector3) { 1.0f, 0.0f, 0.0f }, 0.1f, GREEN);
                DrawSphere((Vector3) { 0.0f, 0.0f, 1.0f }, 0.1f, BLUE);
                DrawSphere((Vector3) { 0.0f, 1.0f, 0.0f }, 0.1f, RED);

                DrawGrid(10, 1.0f);
            EndMode3D();
            DrawFPS(20,20);
            DrawText(TextFormat("Velocity: %0.1f, %0.1f, %0.1f", sphereAcceleration.x, sphereAcceleration.y, sphereAcceleration.z), 20, 40, 15, GREEN);
            DrawText(TextFormat("Position: %0.1f, %0.1f, %0.1f", spherePos.x, spherePos.y, spherePos.z), 20, 60, 15, GREEN);
        EndDrawing();
    }

    wiiuse_cleanup(wiimotes, MAX_WIIMOTES);
    CloseWindow();
    return 0;
}