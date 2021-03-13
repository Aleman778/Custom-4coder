#include "4coder_default_include.cpp"


// TODO(alexander): this is defined here for now
enum {
    EditorMode_Normal = 0,
    EditorMode_Insert = 1,
    // TODO(alexander): maybe add visual mode, but this is same as marking
};
global i32 current_edit_mode = EditorMode_Normal;

#include "4coder_aleman_hooks.cpp"

// NOTE(alexander): define custom binding maps for modal editing
CUSTOM_ID(command_map, mapid_shared);
CUSTOM_ID(command_map, mapid_normal);
CUSTOM_ID(command_map, mapid_insert);
CUSTOM_ID(command_map, mapid_leader);
CUSTOM_ID(command_map, mapid_io);
CUSTOM_ID(command_map, mapid_workspace);

void
set_current_mapid(Application_Links* app, Command_Map_ID mapid) {
    View_ID view = get_active_view(app, 0);
    Buffer_ID buffer = view_get_buffer(app, view, 0);
    Managed_Scope scope = buffer_get_managed_scope(app, buffer);
    Command_Map_ID* map_id_ptr = scope_attachment(app, scope, buffer_map_id, Command_Map_ID);
    *map_id_ptr = mapid;
}

#if !defined(META_PASS)
#include "generated/managed_id_metadata.cpp"
#endif

function void
aleman_4coder_initialize(Application_Links *app,
                     String_Const_u8_Array file_names,
                     i32 override_font_size,
                     b32 override_hinting) {
    Scratch_Block scratch(app);
    
    load_config_and_apply(app, &global_config_arena, override_font_size, override_hinting);
    
    String_Const_u8 bindings_file_name = string_u8_litexpr("bindings.4coder");
    String_Const_u8 mapping = def_get_config_string(scratch, vars_save_string_lit("mapping"));
    
    if (string_match(mapping, string_u8_litexpr("mac-default"))){
        bindings_file_name = string_u8_litexpr("mac-bindings.4coder");
    } else if (OS_MAC && string_match(mapping, string_u8_litexpr("choose"))){
        bindings_file_name = string_u8_litexpr("mac-bindings.4coder");
    }
    
    // if (dynamic_binding_load_from_file(app, &framework_mapping, bindings_file_name)){
        // setup_essential_mapping(&framework_mapping, global_map_id, file_map_id, code_map_id);
    // } else {
        // setup_built_in_mapping(app, mapping, &framework_mapping, global_map_id, file_map_id, code_map_id);
        // setup_essential_mapping(&framework_mapping, global_map_id, file_map_id, code_map_id);
    // }
    
    // open command line files
    String_Const_u8 hot_directory = push_hot_directory(app, scratch);
    for (i32 i = 0; i < file_names.count; i += 1){
        Temp_Memory_Block temp(scratch);
        String_Const_u8 input_name = file_names.vals[i];
        String_Const_u8 full_name = push_u8_stringf(scratch, "%.*s/%.*s",
                                                    string_expand(hot_directory),
                                                    string_expand(input_name));
        Buffer_ID new_buffer = create_buffer(app, full_name, BufferCreate_NeverNew|BufferCreate_MustAttachToFile);
        if (new_buffer == 0){
            create_buffer(app, input_name, 0);
        }
    }
}

CUSTOM_COMMAND_SIG(custom_startup) {
    ProfileScope(app, "custom startup");
    User_Input input = get_current_input(app);
    if (match_core_code(&input, CoreCode_Startup)){
        String_Const_u8_Array file_names = input.event.core.file_names;
        load_themes_default_folder(app);
        Face_Description description = get_face_description(app, 0);
        aleman_4coder_initialize(app, file_names,
                             description.parameters.pt_size,
                             description.parameters.hinting);
        default_4coder_side_by_side_panels(app, file_names);
        b32 auto_load = def_get_config_b32(vars_save_string_lit("automatically_load_project"));
        if (auto_load) {
            load_project(app);
        }
    }

    {
        def_enable_virtual_whitespace = def_get_config_b32(vars_save_string_lit("enable_virtual_whitespace"));
        clear_all_layouts(app);
    }
}

CUSTOM_COMMAND_SIG(to_normal_mode) {
    set_current_mapid(app, mapid_normal);
    current_edit_mode = EditorMode_Normal;
    active_color_table.arrays[ defcolor_cursor ].vals[ 0 ] = 0xffff5533;
    active_color_table.arrays[ defcolor_at_cursor ].vals[ 0 ] = 0xff00aacc;
    active_color_table.arrays[ defcolor_margin_active ].vals[ 0 ] = 0xffff5533;
}

CUSTOM_COMMAND_SIG(to_leader_mode) {
    if (current_edit_mode == EditorMode_Normal) {
        set_current_mapid(app, mapid_leader);

        active_color_table.arrays[ defcolor_cursor ].vals[ 0 ] = 0xff80ff80;
        active_color_table.arrays[ defcolor_at_cursor ].vals[ 0 ] = 0xff293134;
        active_color_table.arrays[ defcolor_margin_active ].vals[ 0 ] = 0xff80ff80;
    }
}

CUSTOM_COMMAND_SIG(to_io_mode) {
    if (current_edit_mode == EditorMode_Normal) {
        set_current_mapid(app, mapid_io);
    }
}

CUSTOM_COMMAND_SIG(to_insert_mode) {
    set_current_mapid(app, mapid_insert);
    current_edit_mode = EditorMode_Insert;
}

function void
aleman_setup_modal_mapping(Mapping *mapping, i64 mapid_global) {
    MappingScope();
    SelectMapping(mapping);
   
    SelectMap(mapid_shared);
    Bind(to_normal_mode, KeyCode_Escape);
    Bind(delete_char, KeyCode_Delete);
    Bind(backspace_char, KeyCode_Backspace);
    Bind(move_left, KeyCode_Left);
    Bind(move_right, KeyCode_Right);
    Bind(move_up, KeyCode_Up);
    Bind(move_down, KeyCode_Down);
    Bind(exit_4coder, KeyCode_F4, KeyCode_Alt);
    Bind(project_fkey_command, KeyCode_F1);
    Bind(project_fkey_command, KeyCode_F2);
    Bind(project_fkey_command, KeyCode_F3);
    Bind(project_fkey_command, KeyCode_F4);
    Bind(project_fkey_command, KeyCode_F5);
    Bind(project_fkey_command, KeyCode_F6);
    Bind(project_fkey_command, KeyCode_F7);
    Bind(project_fkey_command, KeyCode_F8);
    Bind(project_fkey_command, KeyCode_F9);
    Bind(project_fkey_command, KeyCode_F10);
    Bind(project_fkey_command, KeyCode_F11);
    Bind(project_fkey_command, KeyCode_F12);
    Bind(project_fkey_command, KeyCode_F13);
    Bind(project_fkey_command, KeyCode_F14);
    Bind(project_fkey_command, KeyCode_F15);
    Bind(project_fkey_command, KeyCode_F16);
    
    SelectMap(mapid_normal);
    ParentMap(mapid_shared);
    Bind(to_leader_mode, KeyCode_Space);
    Bind(to_insert_mode, KeyCode_F);
    Bind(command_lister, KeyCode_A);
    Bind(delete_char, KeyCode_5);
    Bind(backspace_char, KeyCode_D);
    Bind(move_up, KeyCode_I);
    Bind(move_left, KeyCode_J);
    Bind(move_down, KeyCode_K);
    Bind(move_right, KeyCode_L);
    Bind(move_left_alpha_numeric_or_camel_boundary,  KeyCode_U);
    Bind(move_right_alpha_numeric_or_camel_boundary, KeyCode_O);

    SelectMap(mapid_leader);
    ParentMap(mapid_global);
    Bind(to_normal_mode, KeyCode_Escape);
    Bind(to_io_mode, KeyCode_I);
    BindTextInput(to_normal_mode);

    SelectMap(mapid_io);
    ParentMap(mapid_global);
    Bind(to_normal_mode, KeyCode_Escape);
    Bind(interactive_open_or_new, KeyCode_E);
    BindTextInput(to_normal_mode);

    SelectMap(mapid_insert);
    ParentMap(mapid_shared);
    BindTextInput(write_text_and_auto_indent);
    Bind(to_normal_mode, KeyCode_Home);
}

function void
aleman_setup_essential_mapping(Mapping *mapping, i64 mapid_global, i64 mapid_file, i64 mapid_code) {
    MappingScope();
    SelectMapping(mapping);
    
    SelectMap(mapid_global);
    BindCore(custom_startup, CoreCode_Startup);
    BindCore(default_try_exit, CoreCode_TryExit);
    BindCore(clipboard_record_clip, CoreCode_NewClipboardContents);
    BindMouseWheel(mouse_wheel_scroll);
    BindMouseWheel(mouse_wheel_change_face_size, KeyCode_Control);
    
    // SelectMap(file_id);
    // ParentMap(global_id);
    // BindTextInput(write_text_input);
    BindMouse(click_set_cursor_and_mark, MouseCode_Left);
    BindMouseRelease(click_set_cursor, MouseCode_Left);
    BindCore(click_set_cursor_and_mark, CoreCode_ClickActivateView);
    BindMouseMove(click_set_cursor_if_lbutton);
    
    // SelectMap(mapid_code);
    // ParentMap(mapid_file);
    // BindTextInput(write_text_and_auto_indent);
}



void
custom_layer_init(Application_Links *app) {
    Thread_Context *tctx = get_thread_context(app);

    // NOTE(allen): setup for default framework
    default_framework_init(app);

    // NOTE(alexander): my custom hooks
    aleman_set_all_default_hooks(app);

    mapping_init(tctx, &framework_mapping);

    String_ID mapid_global = vars_save_string_lit("keys_global");
    String_ID mapid_file = vars_save_string_lit("keys_file");
    String_ID mapid_code = vars_save_string_lit("keys_code");

    // NOTE(alexader): use modal mapping
    aleman_setup_modal_mapping(&framework_mapping, mapid_global);

    // NOTE(alexader): use default (non-modal) mapping
    // setup_default_mapping(&framework_mapping, global_map_id, file_map_id, code_map_id);

    aleman_setup_essential_mapping(&framework_mapping, mapid_global, mapid_file, mapid_code);
}
