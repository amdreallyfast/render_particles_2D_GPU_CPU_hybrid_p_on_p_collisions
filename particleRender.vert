#version 440

// Note: The vec2's are in window space (both X and Y on the range [-1,+1])
// Also Note: The vec2s are provided as vec4s on the CPU side and specified as such in the 
// vertex array attributes, but it is ok to only take them as a vec2, as I am doing for this 
// 2D demo.
layout (location = 0) in vec4 pos;  
layout (location = 1) in vec4 vel;  
layout (location = 2) in vec4 netForce;
layout (location = 3) in int collisionCountThisFrame;
layout (location = 4) in float mass;
layout (location = 5) in float radiusOfInfluence;
layout (location = 6) in uint indexOfNodeThatItIsOccupying;
layout (location = 7) in int isActive;

// must have the same name as its corresponding "in" item in the frag shader
smooth out vec4 particleColor;

void main()
{
    // TODO: replace these condition checks with bool->int cast calculation

    if (isActive == 0)
    {
        // invisible (alpha = 0), but "fully transparent" does not mean "no color", it merely 
        // means that the color of this thing will be added to the thing behind it (see Z 
        // adjustment later)
        particleColor = vec4(0.0f, 1.0f, 0.0f, 0.0f);
        gl_Position = vec4(pos.xy, -0.6f, 1.0f);
    }
    else
    {
        // attempting to perform conditional elimination to improve performance, but I'm not getting a single frame back, so I don't think that this is my bottleneck
        float red = 0.0f;       // high velocity
        float green = 0.0f;     // medium velocity
        float blue = 0.0f;      // low velocity

        float min = 0;
        float mid = 15;
        float max = 30;
    
        // I know that "collision count" is an int, but make a float out of it for the sake of 
        // these calculation
        float value = collisionCountThisFrame;

        float fractionLowMid = (value - min) / (mid - min);
        float fractionMidHigh = (value - mid) / (max - mid);

        float collisionCountLow = float(value < mid);

        // replaced if (value < mid) { blue-green } else { green-red } conditions 
        red = (1 - collisionCountLow) * fractionMidHigh;
        green = ((1 - collisionCountLow) * (1 - fractionMidHigh)) + (collisionCountLow * fractionLowMid);
        blue = (collisionCountLow * (1 - fractionLowMid));

        particleColor = vec4(red, green, blue, 1.0f);
        
        gl_Position = vec4(pos.xy, -0.7f, 1.0f);
    }
}

