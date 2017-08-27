// ImGui - standalone example application for Glfw + OpenGL 2, using fixed pipeline
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.

#include "imgui/imgui.h"
#include "imgui_backend/imgui_impl_glfw.h"
#include <stdio.h>
#include <GLFW/glfw3.h>

extern "C"{
#include "configure/stdafx.h"

#include "zhwkre/bss.h"
#include "zhwkre/list.h"
#include "zhwkre/concurrent.h"

#include "protocol/protocol.h"

#include "clinet/clinet.h"
#include "zhwkre/serialization.h"
#include "permissionctl/permissionctl.h"
}

#define PUSH(num) (((ui)1)<<(num))
#define SETSTAT(stat) (curr_status |= (stat))
#define CLRSTAT(stat) (curr_status &= ~(stat))
#define CHKSTAT(stat) (curr_status & (stat))
#define RSTSTAT (curr_status & ((ui)0))

#define SETDIRT(stat) (curr_dirty |= (stat))
#define CLRDIRT(stat) (curr_dirty &= ~(stat))
#define CHKDIRT(stat) (curr_dirty & (stat))
#define RSTDIRT (curr_dirty & ((ui)0))

typedef unsigned int ui;

ui curr_status = 0;
ui curr_dirty = 0;
// some_consts
const ui uistat_login = PUSH(0);
const ui uistat_process_login = PUSH(1);
const ui uistat_syncdata = PUSH(2);
const ui uistat_main = PUSH(3);
const ui uistat_alterpass = PUSH(4);
const ui uistat_syncgroup = PUSH(5);
const ui uistat_showgroup = PUSH(6);
const ui uistat_showuser = PUSH(7);
const ui uistat_syncuser = PUSH(8);
const ui uistat_level1 = PUSH(9);
const ui uistat_level2 = PUSH(10);
const ui uistat_level3 = PUSH(11);
const ui uistat_l1append = PUSH(12);
const ui uistat_l2append = PUSH(13);
const ui uistat_l3append = PUSH(14);
const ui uistat_alterowner = PUSH(15);
const ui uistat_altergroup = PUSH(16);
const ui uistat_alterperm = PUSH(17);

static void error_callback(int error, const char* description){
    fprintf(stderr, "Error %d: %s\n", error, description);
}


int main(int argc, char** argv)
{
    // Setup window
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        return 1;
    GLFWwindow* window = glfwCreateWindow(1280, 720, "CSMcli -- powered by ImGui/OpenGL", NULL, NULL);
    glfwMakeContextCurrent(window);

    // Setup ImGui binding
    ImGui_ImplGlfw_Init(window, true);
    { // fonts reference
        // Load Fonts
        // (there is a default font, this is only if you want to change it. see extra_fonts/README.txt for more details)
        //ImGuiIO& io = ImGui::GetIO();
        //io.Fonts->AddFontDefault();
        //io.Fonts->AddFontFromFileTTF("../../extra_fonts/Cousine-Regular.ttf", 15.0f);
        //io.Fonts->AddFontFromFileTTF("../../extra_fonts/DroidSans.ttf", 16.0f);
        //io.Fonts->AddFontFromFileTTF("../../extra_fonts/ProggyClean.ttf", 13.0f);
        //io.Fonts->AddFontFromFileTTF("../../extra_fonts/ProggyTiny.ttf", 10.0f);
        //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    }
    ImVec4 clear_color = ImColor(114, 144, 154);
    // define basic globalvars
        char usernamebuffer[256],passwordbuffer[256];
        char serverbuffer[256];
        ui uid=9999,gid=9999;
        ui destuid=9999,destgid=9999;
        qListDescriptor *grouplist=NULL;
        qListDescriptor *userlist = NULL;
        qListDescriptor *data = NULL;
        Level1Entry *currl1e=NULL;Level2Entry *currl2e=NULL;Level3Entry *currl3e=NULL;
        Level1Entry l1buffer;Level2Entry l2buffer;Level3Entry l3buffer;
        unsigned char destperm[3];
        // setup basic envirs
    {
        SETSTAT(uistat_login);
        memset(usernamebuffer,0,256);
        memset(passwordbuffer,0,256);
        memset(serverbuffer,0,256);
        memset(&l1buffer,0,sizeof(l1buffer));
        memset(&l2buffer,0,sizeof(l2buffer));
        memset(&l3buffer,0,sizeof(l3buffer));
        memcpy(destperm,umask,3*sizeof(unsigned char));
        qList_initdesc(ui_notifier);
        qList_initdesc(network_notifier);
        ui_noti_lock = qMutex_constructor();
        net_noti_lock = qMutex_constructor();
        qRun(handle_network,NULL);// start process
    }

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        ImGui_ImplGlfw_NewFrame();
        // handle events
        LOCKUI;
        if(ui_notifier.size != 0){
            Messeage *msg = (Messeage*)ui_notifier.head->data;
            switch(msg->qid){
                case 0:
                {
                    LoginReply r = qDisassembleLoginReply(msg->payload);
                    if(!(r.errNo)){
                        // succ
                        uid = r.userId;
                        gid = r.groupId;
                        RSTSTAT;
                        SETSTAT(uistat_syncdata);
                        SETDIRT(uistat_syncdata);
                    }else{
                        ImGui::BeginPopup("Fatal Error##on_authentication_error");
                        ImGui::Text("Autentication Error.");
                        ImGui::EndPopup();
                        RSTSTAT;
                        SETSTAT(uistat_login);
                    }
                }
                break;
                case 1:
                {
                    AlterPassReply r = qDisassembleAlterPassReply(msg->payload);
                    if(!(r.errNo)){
                        CLRSTAT(uistat_alterpass);
                        ImGui::BeginPopup("Success##on_password_alter_succ");
                        ImGui::Text("Password Alternation Success.");
                        ImGui::EndPopup();
                    }else{
                        CLRSTAT(uistat_alterpass);
                        ImGui::BeginPopup("Failed##on_password_alter_fail");
                        ImGui::Text("Password Alternation Failed.");
                        ImGui::EndPopup();
                    }
                }
                break;
                case 2:
                {
                    grouplist = qDisassembleListGroupReply(msg->payload);
                    if(grouplist->size != 0){
                        CLRSTAT(uistat_syncgroup);
                        SETSTAT(uistat_showgroup);
                    }else{
                        CLRSTAT(uistat_syncgroup);
                        ImGui::BeginPopup("Failed##on_group_sync_failed");
                        ImGui::Text("List Group Failed -- Permission DENIED.");
                        ImGui::EndPopup();
                    }
                }
                break;
                case 3:
                {
                    AlterGroupReply r = qDisassembleAlterGroupReply(msg->payload);
                    if(!(r.errNo)){
                        ImGui::BeginPopup("Success##on_group_alter_success");
                        ImGui::Text("Alter Group Success.");
                        ImGui::EndPopup();
                    }else{
                        ImGui::BeginPopup("Failed##on_group_alter_failed");
                        ImGui::Text("Alter Group Failed.Permission Denied.");
                        ImGui::EndPopup();
                    }
                }
                break;
                case 4:
                {
                    RemoveUserReply r = qDisassembleRemoveUserReply(msg->payload);
                    if(!(r.errNo)){
                        CLRSTAT(uistat_showuser);
                        ImGui::BeginPopup("Success##on_user_remove_success");
                        ImGui::Text("Remove User Success.");
                        ImGui::EndPopup();
                    }else{
                        ImGui::BeginPopup("Failed##on_user_remove_failed");
                        ImGui::Text("Remove User Failed.");
                        ImGui::EndPopup();
                    }
                }
                break;
                case 5:
                {
                    RemoveGroupReply r = qDisassembleRemoveGroupReply(msg->payload);
                    if(!(r.errNo)){
                        CLRSTAT(uistat_showgroup);
                        ImGui::BeginPopup("Success##on_group_remove_success");
                        ImGui::Text("Remove Group Success.");
                        ImGui::EndPopup();
                    }else{
                        ImGui::BeginPopup("Failed##on_group_remove_failed");
                        ImGui::Text("Remove Group Failed.");
                        ImGui::EndPopup();
                    }
                }
                break;
                case 6:
                {
                    userlist = qDisassembleListUserReply(msg->payload);
                    if(userlist->size){
                        CLRSTAT(uistat_syncuser);
                        SETSTAT(uistat_showuser);
                    }else{
                        CLRSTAT(uistat_syncuser);
                        ImGui::BeginPopup("Failed##on_sync_user_failed");
                        ImGui::Text("List User Failed. Permission DENIED.");
                        ImGui::EndPopup();
                    }
                }
                break;
                case 10:
                {
                    qListDescriptor tmpunser = qDisassembleSyncDataReply(msg->payload);
                    if(tmpunser.size != 0){
                        data = (qListDescriptor*)qUnserialize(tmpunser,YES_IT_IS_A_LIST);
                    }else{
                        q__List_initdesc(data);
                    }
                    CLRSTAT(uistat_syncdata);
                    SETSTAT(uistat_main);
                }
                break;
                case 11:
                {
                    AppendDataReply r = qDisassembleAppendDataReply(msg->payload);
                    if(!(r.errNo)){
                        if(currl2e != NULL){
                            CLRSTAT(uistat_l3append);
                            setpe(l3buffer.pe,uid,gid,r.entryId,umask);
                            qList_push_back(currl2e->ld,l3buffer);
                        }else if(currl1e!=NULL){
                            CLRSTAT(uistat_l2append);
                            setpe(l2buffer.pe,uid,gid,r.entryId,umask);
                            qList_initdesc(l2buffer.ld);
                            qList_push_back(currl1e->ld,l2buffer);
                        }else{
                            CLRSTAT(uistat_l1append);
                            setpe(l1buffer.pe,uid,gid,r.entryId,umask);
                            qList_initdesc(l1buffer.ld);
                            qList_push_back(*data,l1buffer);
                        }
                    }else{
                        if(currl2e != NULL){
                            CLRSTAT(uistat_l3append);
                        }else if(currl1e!=NULL){
                            CLRSTAT(uistat_l2append);
                        }else{
                            CLRSTAT(uistat_l1append);
                        }
                        ImGui::BeginPopup("FAIL##on_append_entry_fail");
                        ImGui::Text("Append Entry failed. Error Number is %d",r.errNo);
                    }
                }
                break;
                case 12:
                {
                    RemoveDataReply r = qDisassembleRemoveDataReply(msg->payload);
                    if(!(r.errNo)){
                        if(currl3e != NULL){
                            CLRSTAT(uistat_level3);
                            qList_foreach(currl2e->ld,iter){
                                Level3Entry *ref = (Level3Entry*)iter->data;
                                if(ref->pe.entryid == currl3e->pe.entryid){
                                    qList_erase_elem(currl2e->ld,iter);
                                    break;
                                }
                            }
                            currl3e = NULL;
                        }else if(currl2e!=NULL){
                            CLRSTAT(uistat_level2);
                            qList_foreach(currl1e->ld,iter){
                                Level2Entry *ref = (Level2Entry *)iter->data;
                                if(ref->pe.entryid == currl2e->pe.entryid){
                                    qList_destructor(ref->ld);
                                    qList_erase_elem(currl1e->ld,iter);
                                    break;
                                }
                            }
                            currl2e = NULL;
                        }else if(currl1e!=NULL){
                            CLRSTAT(uistat_level1);
                            qList_foreach(*data,iter){
                                Level1Entry *ref = (Level1Entry*) iter->data;
                                if(ref->pe.entryid == currl1e->pe.entryid){
                                    qList_foreach(ref->ld,iiter){
                                        Level2Entry *rref = (Level2Entry*) iiter->data;
                                        qList_destructor(rref->ld);
                                    }
                                    qList_destructor(ref->ld);
                                    qList_erase_elem(*data,iter);
                                    break;
                                }
                            }
                            currl1e = NULL;
                        }
                    }else{
                        ImGui::BeginPopup("FAIL##on_remove_entry_failed");
                        ImGui::Text("Remove Entry failed. Error code is %d.",r.errNo);
                        ImGui::EndPopup();
                    }
                }
                break;
                case 13:
                {
                    AlterDataReply r = qDisassembleAlterDataReply(msg->payload);
                    if(!(r.errNo)){
                        if(currl3e != NULL){
                            memcpy(&(currl3e->data),&(l3buffer.data),sizeof(Level3));
                        }else if(currl2e!=NULL){
                            memcpy(&(currl2e->data),&(l2buffer.data),sizeof(Level2));
                        }else if(currl1e != NULL){
                            memcpy(&(currl1e->data),&(l1buffer.data),sizeof(Level1));
                        }
                    }else{
                        ImGui::BeginPopup("FAIL##on_alter_entry_failed");
                        ImGui::Text("Alter Entry failed. Error code is %d.",r.errNo);
                        ImGui::EndPopup();
                    }
                }
                break;
                case 20:
                {
                    AlterEntryOwnerReply r = qDisassembleAlterEntryOwnerReply(msg->payload);
                    CLRSTAT(uistat_alterowner);
                    if(!(r.errNo)){
                        if(currl3e != NULL){
                            currl3e->pe.ownerid = destuid;
                        }else if(currl2e!=NULL){
                            currl2e->pe.ownerid = destuid;
                        }else if(currl1e != NULL){
                            currl1e->pe.ownerid = destuid;
                        }
                    }else{
                        ImGui::BeginPopup("FAIL##on_alter_owner_fail");
                        ImGui::Text("Alter Entry Owner failed. Error code is %d",r.errNo);
                        ImGui::EndPopup();
                    }
                }
                break;
                case 21:
                {
                    AlterEntryGroupReply r = qDisassembleAlterEntryGroupReply(msg->payload);
                    CLRSTAT(uistat_altergroup);
                    if(!(r.errNo)){
                        if(currl3e != NULL){
                            currl3e->pe.groupid = destgid;
                        }else if(currl2e!=NULL){
                            currl2e->pe.groupid = destgid;
                        }else if(currl1e != NULL){
                            currl1e->pe.groupid = destgid;
                        }
                    }else{
                        ImGui::BeginPopup("FAIL##on_alter_group_fail");
                        ImGui::Text("Alter Entry Group failed. Error code is %d",r.errNo);
                        ImGui::EndPopup();
                    }
                }
                break;
                case 22:
                {
                    AlterEntryPermissionReply r = qDisassembleAlterEntryPermissionReply(msg->payload);
                    CLRSTAT(uistat_alterowner);
                    if(!(r.errNo)){
                        if(currl3e != NULL){
                            memcpy(currl3e->pe.permission,destperm,3*sizeof(unsigned char));
                        }else if(currl2e!=NULL){
                            memcpy(currl2e->pe.permission,destperm,3*sizeof(unsigned char));
                        }else if(currl1e != NULL){
                            memcpy(currl1e->pe.permission,destperm,3*sizeof(unsigned char));
                        }
                    }else{
                        ImGui::BeginPopup("FAIL##on_alter_owner_fail");
                        ImGui::Text("Alter Entry Owner failed. Error code is %d",r.errNo);
                        ImGui::EndPopup();
                    }
                }
                break;
                default:
                {
                    ImGui::BeginPopup("UNEXPECTED BEHAVIOR##on_receive_unknown_qid");
                    ImGui::Text("Program control flow reached unexpected part. qid is %d",msg->qid);
                    ImGui::EndPopup();
                }
                break;
            }
            qbss_destructor(msg->payload);
        }
        UNLOCKUI;
        /* references
        {
            // 1. Show a simple window
            // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
            {
                static float f = 0.0f;
                ImGui::Text("Hello, world!");
                ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
                ImGui::ColorEdit3("clear color", (float*)&clear_color);
                if (ImGui::Button("Test Window")) show_test_window ^= 1;
                if (ImGui::Button("Another Window")) show_another_window ^= 1;
                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            }

            // 2. Show another simple window, this time using an explicit Begin/End pair
            if (show_another_window)
            {
                ImGui::SetNextWindowSize(ImVec2(200,100), ImGuiSetCond_FirstUseEver);
                ImGui::Begin("Another Window", &show_another_window);
                ImGui::Text("Hello");
                ImGui::End();
            }

            // 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
            if (show_test_window)
            {
                ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
                ImGui::ShowTestWindow(&show_test_window);
            }
        }*/

        if(CHKSTAT(uistat_login)){
            ImGui::Begin("Login or Register");
            ImGui::Text("Server format: 127.0.0.1:1992");
            ImGui::InputText("Server",serverbuffer,256);
            ImGui::InputText("Username",usernamebuffer,256);
            ImGui::InputText("Password",passwordbuffer,256,ImGuiInputTextFlags_Password);
            if(ImGui::Button("Login")){
                fprintf(stderr,"Attempt login with %s + %s to %s\n",usernamebuffer,passwordbuffer,serverbuffer);
                // TODO:send request to network notifier
                CLRSTAT(uistat_login);
                SETSTAT(uistat_process_login);
            }
            ImGui::SameLine();
            if(ImGui::Button("Register")){
                fprintf(stderr,"Attempt register with %s + %s to %s\n",usernamebuffer,passwordbuffer,serverbuffer);
                // TODO:send request to network handle
                CLRSTAT(uistat_login);
                SETSTAT(uistat_process_login);
            }
            ImGui::End();
        }
        if(CHKSTAT(uistat_process_login)){
            ImGui::Begin("Please Wait");
            ImGui::Text("Connecting to server..");
            ImGui::End();
        }

        // Rendering
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::Render();
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplGlfw_Shutdown();
    glfwTerminate();

    return 0;
}
