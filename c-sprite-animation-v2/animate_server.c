
#include <animate/animate.h>
#include <stdlib.h>

int main(int argc, char** argv, char** envp) {

    // Reserved for future server configuration and environment handling.
    (void)argc;
    (void)argv;
    (void)envp;

    struct canvas* canvas = animate_create_canvas(100,100,0);
    if (canvas == NULL) {
        return EXIT_FAILURE;
    }
    animate_destroy_canvas(canvas);

    //Connection establishment (signals and FIFO )

    //start the server and print its pid 

    //listen for the signals from clients 

        //create FIFO 

    //send the signal back to the client 
    
    return EXIT_SUCCESS;
}
