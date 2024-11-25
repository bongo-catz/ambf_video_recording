//==============================================================================
/*
    Software License Agreement (BSD License)
    Copyright (c) 2019-2024, AMBF
    (https://github.com/WPI-AIM/ambf)

    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following
    disclaimer in the documentation and/or other materials provided
    with the distribution.

    * Neither the name of authors nor the names of its contributors may
    be used to endorse or promote products derived from this software
    without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
    COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
    ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

    \author    <amunawa2@jh.edu>
    \author    Adnan Munawar
*/
//==============================================================================

#include "video_recording.h"
#include <boost/program_options.hpp>
#include <filesystem>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <ctime>

using namespace std;
namespace fs = std::filesystem;

//------------------------------------------------------------------------------
// DECLARED FUNCTIONS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
namespace p_opt = boost::program_options;

string g_current_filepath;

afSimulatorVideoRecorderPlugin::afSimulatorVideoRecorderPlugin()
{

}

//------------------------------------------------------------------------------
// Function to create a new subdirectory inside "Simulator_Recordings" directory
//------------------------------------------------------------------------------
string createNewDirectory(const string& baseDir) {
    // Create the overarching "Simulator_Recordings" directory if it does not exist
    string baseDirectory = "Simulator_Recordings";
    if (!fs::exists(baseDirectory)) {
        if (!fs::create_directory(baseDirectory)) {
            cerr << "Failed to create base directory: " << baseDirectory << endl;
            throw runtime_error("Base directory creation failed");
        }
    }
    // Get current time for a unique directory name
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t_now), "%Y%m%d_%H%M%S");
    string newDir = baseDirectory + "/" + ss.str();
    // Create the directory inside the base directory
    if (!fs::create_directories(newDir)) {
        cerr << "Failed to create directory: " << newDir << endl;
        throw runtime_error("Directory creation failed");
    }
    return newDir;
}

int afSimulatorVideoRecorderPlugin::init(int argc, char **argv, const afWorldPtr a_afWorld)
{
    m_worldPtr = a_afWorld;

    m_camera = m_worldPtr->getCameras()[0];

    // We can specify the video resolution or copy it from current camera.
    m_width = m_camera->m_width;
    m_height = m_camera->m_height;
    // m_width = 640;
    // m_height = 480;

    m_frameBuffer = cFrameBuffer::create();
    m_frameBuffer->setup(m_camera->getInternalCamera(), m_width, m_height, true, true);

    m_image = cImage::create();

    // Create a new directory to save the video under Simulator_Recordings
    string saveDirectory = createNewDirectory("Simulator_Recordings");

    // ffmpeg settings, these can be changed to balance speed, size and quality
    time_t now = time(0);
    char* dt = ctime(&now);
    string size_str = to_string(m_width) + "x" + to_string(m_height);
    m_video_filename = saveDirectory + "/" + m_camera->getName() + "_" + to_string(int(m_worldPtr->getSystemTime())) + ".mp4";
    string cmd = "ffmpeg";
    cmd += " -r 60";
    cmd += " -f rawvideo";
    cmd += " -pix_fmt rgba";
    cmd += " -s " + size_str;
    cmd += " -i -";
    cmd += " -threads 0";
    cmd += " -preset fast";
    cmd += " -y";
    cmd += " -pix_fmt yuv420p";
    cmd += " -crf 21"; // Major parameter to tweak video compression and output size
    cmd += " -vf vflip";
    cmd += " " + m_video_filename;

    try{
        m_ffmpeg = popen(cmd.c_str(), "w");
        cerr << "INFO! SUCCESSFULLY LOADED VIDEO WRITER PLUGIN \n" << endl;
    }
    catch (exception e){
        cerr << e.what() << endl;
        return -1;
    }

    return 1;
}

void afSimulatorVideoRecorderPlugin::keyboardUpdate(GLFWwindow *a_window, int a_key, int a_scancode, int a_action, int a_mods)
{

}

void afSimulatorVideoRecorderPlugin::mouseBtnsUpdate(GLFWwindow *a_window, int a_button, int a_action, int a_modes)
{

}

void afSimulatorVideoRecorderPlugin::mousePosUpdate(GLFWwindow *a_window, double x_pos, double y_pos) {}

void afSimulatorVideoRecorderPlugin::mouseScrollUpdate(GLFWwindow *a_window, double x_pos, double y_pos)
{

}

void afSimulatorVideoRecorderPlugin::graphicsUpdate()
{
    try{
        m_frameBuffer->renderView();
        m_frameBuffer->copyImageBuffer(m_image);
        fwrite(m_image->getData(), m_image->getBytesPerPixel() * m_width * m_height, 1, m_ffmpeg);
    }
    catch (exception e){
        cerr << e.what() << endl;
    }

    static bool save_first_frame_img = true;
    if(save_first_frame_img){
        m_image->saveToFile(m_camera->getName() + ".png");
        save_first_frame_img = false;
    }
}

void afSimulatorVideoRecorderPlugin::physicsUpdate(double dt)
{

}

void afSimulatorVideoRecorderPlugin::reset()
{

}

bool afSimulatorVideoRecorderPlugin::close()
{
    if (m_ffmpeg){
        pclose(m_ffmpeg);
        cerr << "INFO! CLOSING FFMPEG FILE \n";
    }
    return true;
}
