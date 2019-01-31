/* comment from youtube video
 Watch out for:
 -use of a non-standard preprocessor directive (pragma once),
 -use of user-defined type (aistring) before including the library which defines it,
 -inconsistent naming of struct members (in Vertex your members start with caps, in Texture they don't),
 -heavy use of copy assignment instead of pointer types,
 -unnecessary use of a newline before an opening curly brace.﻿
 */
// what is pragma once?
// code adapted from https://www.youtube.com/watch?v=ZbnEMM7vwmU

// Std. Includes
#include <string>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GL includes
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "Renderer.h"
#include "FramebufferReader.h"

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

// Other Libs
#include "SOIL/SOIL.h"
#include <jpeglib.h>

#include <iostream>
#include <stdlib.h>
#include "fcgio.h"
#include <algorithm>
#include <iterator>
#include <vector>

using namespace std;

// http://www.martinbroadhurst.com/how-to-split-a-string-in-c.html
void split(const std::string& str, std::vector<std::string>& cont, char delim);

int main(int argc, char **argv) {
	// Backup the stdio streambufs
    streambuf * cin_streambuf  = cin.rdbuf();
    streambuf * cout_streambuf = cout.rdbuf();
    streambuf * cerr_streambuf = cerr.rdbuf();

    FCGX_Request request;

    FCGX_Init();
    FCGX_InitRequest(&request, 0, 0);
    
    Renderer renderer(800, 600);

    Scene scene;
    Camera camera(glm::vec3( 0.0f, 0.0f, 0.0f ), 45.0f);
    // Model ourModel( "res/models/XYZ_RGB_dragon/XYZ_RGB_dragon.obj" );
    // Model ourModel( "res/models/cube/cube.obj" );
    Model ourModel( "res/models/happy_recon/happy_final.obj" );
    
    scene.add( &ourModel );

    float m[] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

    FramebufferReader framebufferReader(3, 800, 600);

    // Game loop
    // while ( true ) {        
    while (FCGX_Accept_r(&request) == 0) {
        fcgi_streambuf cin_fcgi_streambuf(request.in);
        fcgi_streambuf cout_fcgi_streambuf(request.out);
        fcgi_streambuf cerr_fcgi_streambuf(request.err);

        cin.rdbuf(&cin_fcgi_streambuf);
        cout.rdbuf(&cout_fcgi_streambuf);
        cerr.rdbuf(&cerr_fcgi_streambuf);    

        char * uri = FCGX_GetParam("REQUEST_URI", request.envp);
        std::string uri_str(uri);

        // get route
        char *temp1;
        if (uri[0] == '/') {
            temp1 = &uri[1];
        }
        string temp2(temp1);
        int i = temp2.find('/');
        temp2 = temp2.substr(i+1,temp2.length());
        i = temp2.find('/');
        string route = temp2.substr(0,i);

        std::vector<std::string> words;
        split(temp2.substr(i+1,temp2.length()), words, ',');
        
        if ( route.compare("render") == 0 ) {
            for (unsigned int j=0; j<16; j++) {
                std::cout << (words[j]) << " ";
                m[j] = std::stod(words[j]);
            }
            camera.fov = std::stod(words[16]);
            renderer.useTexture = std::stoi(words[17]);

            // camera.fov = 45.0f;
            // renderer.useTexture = 1;

            // std::cout << "Content-type: text/html\r\n\r\n" << "1" << route;
        } else if ( route.compare("light") == 0 ) {
                        // std::cout << "Content-type: text/html\r\n\r\n" << "ERROR: Invalid route: ";

            int i = 0;
            renderer.useLight[0] = std::stoi(words[i]);
            i++;
            for (int j=0; j<3; j++) {
                std::cout << words[i+j];
                renderer.lightPosition[0][j] = std::stod(words[i+j]);
            }
            i+=3;

            renderer.useLight[1] = std::stoi(words[i]);
            i++;
            for (int j=0; j<3; j++) {
                std::cout << words[i+j];
                renderer.lightPosition[1][j] = std::stod(words[i+j]);
            }
            i+=3;
        } else {
            std::cout << "Content-type: text/html\r\n\r\n" << "ERROR: Invalid route: " << route << std::endl;
        }

        ourModel.modelMatrix = glm::make_mat4(m);

        renderer.render( &scene, &camera );

        std::cout << "Content-type: image/png\r\n\r\n";
        framebufferReader.writeFrameToCout();

        // std::cout << "Content-type: text/html\r\n\r\n" << route;
        std::cout << std::endl;
        // std::cin.ignore();
    }
    
    renderer.close();
    
    // restore stdio streambufs
    cin.rdbuf(cin_streambuf);
    cout.rdbuf(cout_streambuf);
    cerr.rdbuf(cerr_streambuf);

    return 0;
}

// http://www.martinbroadhurst.com/how-to-split-a-string-in-c.html
void split(const std::string& str, std::vector<std::string>& cont, char delim = ' ')
{
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delim)) {
        cont.push_back(token);
    }
}