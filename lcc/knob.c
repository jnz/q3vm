#define KNOB_IMPLEMENTATION
#include "knob.h"


MAIN(lcc_build){

    const char* program = knob_shift_args(&argc,&argv);
    char* subcommand = NULL;
    if(argc <= 0){
        subcommand = "build";
    } else {
        subcommand = (char*)knob_shift_args(&argc,&argv);
    }

    char cwd[260] = {0};
    getcwd(cwd,260);
    compiler_names[COMPILER_COSMOCC][0] = knob_temp_sprintf("%s/cosmocc/bin/cosmocc",cwd);
    #define NUM_EXE 3
    const char* src_dir[NUM_EXE] = {"./src","./cpp","./etc"};
    const char* exe_name[NUM_EXE] = {"q3rcc.com","q3cpp.com","lcc.com"};
    for(int i =0; i < NUM_EXE;++i){
        Knob_Cmd cmd = {0};
        Knob_Config config = {0};
        knob_config_init(&config);
        config.compiler = COMPILER_COSMOCC;

        Knob_File_Paths files = {0};
        knob_da_mult_append(&files,
            src_dir[i]
        );
        config.build_to = "."PATH_SEP"build";
        knob_config_add_files(&config,files.items,files.count);
        files.count = 0;
        knob_da_mult_append(&files,
            src_dir[i]
        );
        knob_config_add_includes(&config,files.items,files.count);
        knob_config_add_c_flag(&config,"-fno-strict-aliasing");
        knob_config_add_c_flag(&config,"-fmessage-length=0");
        knob_config_add_c_flag(&config,"-fno-common");
        knob_config_add_c_flag(&config,"-Wall");
        knob_config_add_c_flag(&config,"-Wno-misleading-indentation");
        knob_config_add_c_flag(&config,"-O0");
        knob_config_add_define(&config,"-DCOSMOPOLITAN");


        Knob_File_Paths out_files = {0};
        if(!knob_config_build(&config,&out_files,0))return 1;
        

        cmd.count = 0;

        knob_cmd_add_compiler(&cmd,&config);
        knob_cmd_add_includes(&cmd,&config);
        for(int i =0; i < out_files.count;++i){
            knob_cmd_append(&cmd,out_files.items[i]);
        }

        knob_cmd_append(&cmd,"-o",knob_temp_sprintf("./build/%s",exe_name[i]));
        knob_cmd_append(&cmd,"-lm");

        Knob_String_Builder render = {0};
        knob_cmd_render(cmd,&render);
        knob_log(KNOB_INFO,"CMD: %s",render.items);
        if(!knob_cmd_run_sync(cmd)) return 1;
    }

    return 0;
}