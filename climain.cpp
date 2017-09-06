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
#include "zhwkre/utils.h"

#include "protocol/protocol.h"

#include "clinet/clinet.h"
#include "zhwkre/serialization.h"
#include "permissionctl/permissionctl.h"
#include "zhwkre/alg.h"
}

#define PUSH(num) (((ui)1)<<(num))
#define SETSTAT(stat) (curr_status |= (stat))
#define CLRSTAT(stat) (curr_status = curr_status & (~(stat)))
#define CHKSTAT(stat) (curr_status & (stat))
#define RSTSTAT (curr_status & ((ui)0))

#define SETDIRT(stat) (curr_dirty |= (stat))
#define CLRDIRT(stat) (curr_dirty &= ~(stat))
#define CHKDIRT(stat) (curr_dirty & (stat))
#define RSTDIRT (curr_dirty & ((ui)0))

#define GUICOLUMNNEXT(...) ImGui::Text(__VA_ARGS__);ImGui::NextColumn()

typedef unsigned int ui;

float fepsilon = 0.000001f;

ui curr_status = 0;
ui curr_dirty = 0;
// some_consts
const ui uistat_login = PUSH(0); // --
const ui uistat_process_login = PUSH(1); // --
const ui uistat_syncdata = PUSH(2); // --
const ui uistat_main = PUSH(3); // --
const ui uistat_alterpass = PUSH(4); // --
const ui uistat_syncgroup = PUSH(5);  // --
const ui uistat_showgroup = PUSH(6); // --
const ui uistat_showuser = PUSH(7); // --
const ui uistat_syncuser = PUSH(8); // --
const ui uistat_level1 = PUSH(9); // --
const ui uistat_level2 = PUSH(10); // --
const ui uistat_level3 = PUSH(11); // --
const ui uistat_l1append = PUSH(12); // --
const ui uistat_l2append = PUSH(13); // --
const ui uistat_l3append = PUSH(14); // --
const ui uistat_alterowner = PUSH(15); // --
const ui uistat_altergroup = PUSH(16); // --
const ui uistat_alterperm = PUSH(17); // --
const ui uistat_admin = PUSH(18); // --
const ui uistat_setgroup = PUSH(19); // --
const ui uistat_lookup = PUSH(20);
const ui uistat_statistic = PUSH(21);

static void error_callback(int error, const char* description){
    fprintf(stderr, "Error %d: %s\n", error, description);
}

int int_decre_sort(qListIterator a,qListIterator b){
    SoldNumSort *ia=(SoldNumSort*)a->data,*ib=(SoldNumSort*)b->data;
    return (ia->sorter)<(ib->sorter);
}

int float_decre_sort(qListIterator a,qListIterator b){
    SoldMonSort *ia=(SoldMonSort*)a->data,*ib=(SoldMonSort*)b->data;
    return (ia->sorter)<(ib->sorter);
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
        char serverbuffer[256];char shutpassbuffer[256];
        char origpassbuffer[256],alterpassbuffer[256];
        char alterpermbuffer[10];
        char lookupbuffer[256];
        ui lookupstat = 0,stistat = 0;
        ui uid=9999,gid=9999;
        ui destuid=9999,destgid=9999;
        ui destrmuid = 9999,destrmgid=9999;
        int tmpcolorR=0,tmpcolorG=0,tmpcolorB=0;
        float picker=0.0;
        qListDescriptor *grouplist=NULL;
        qListDescriptor *userlist = NULL;
        qListDescriptor *data = NULL;
        Level1Entry *currl1e=NULL;Level2Entry *currl2e=NULL;Level3Entry *currl3e=NULL;
        Level1Entry l1buffer;Level2Entry l2buffer;Level3Entry l3buffer;
        unsigned char destperm[3];
        const char* dispstr=NULL;
        // setup basic envirs
    {
        SETSTAT(uistat_login);
        memset(usernamebuffer,0,256);
        memset(passwordbuffer,0,256);
        memset(serverbuffer,0,256);
        memset(origpassbuffer,0,256);
        memset(alterpassbuffer,0,256);
        memset(alterpermbuffer,0,10);
        memset(shutpassbuffer,0,256);
        memset(lookupbuffer,0,256);
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
                    CLRSTAT(uistat_process_login);
                    if(!(r.errNo)){
                        // succ
                        uid = r.userId;
                        gid = r.groupId;
                        RSTSTAT;
                        SETSTAT(uistat_syncdata);
                        SETDIRT(uistat_syncdata);
                    }else{
                        dispstr = "Autentication Error.";
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
                        memcpy(passwordbuffer,alterpassbuffer,256);
                        dispstr = "Password Alternation Success.";
                    }else{
                        CLRSTAT(uistat_alterpass);
                        dispstr = "Password Alternation Failed.";
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
                        dispstr = ("List Group Failed -- Permission DENIED.");
                    }
                }
                break;
                case 3:
                {
                    AlterGroupReply r = qDisassembleAlterGroupReply(msg->payload);
                    if(!(r.errNo)){
                        dispstr = ("Alter Group Success.");
                    }else{
                        dispstr = ("Alter Group Failed.Permission Denied.");
                    }
                }
                break;
                case 4:
                {
                    RemoveUserReply r = qDisassembleRemoveUserReply(msg->payload);
                    if(!(r.errNo)){
                        CLRSTAT(uistat_showuser);
                        dispstr = ("Remove User Success.");
                    }else{
                        dispstr = ("Remove User Failed.");
                    }
                }
                break;
                case 5:
                {
                    RemoveGroupReply r = qDisassembleRemoveGroupReply(msg->payload);
                    if(!(r.errNo)){
                        CLRSTAT(uistat_showgroup);
                        dispstr = ("Remove Group Success.");
                    }else{
                        dispstr = ("Remove Group Failed.");
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
                        dispstr = ("List User Failed. Permission DENIED.");
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
                        if(CHKSTAT(uistat_l3append)){
                            CLRSTAT(uistat_l3append);
                            setpe(l3buffer.pe,uid,gid,r.entryId,umask);
                            qList_push_back(currl2e->ld,l3buffer);
                        }else if(CHKSTAT(uistat_l2append)){
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
                        if(CHKSTAT(uistat_l3append)){
                            CLRSTAT(uistat_l3append);
                        }else if(CHKSTAT(uistat_l2append)){
                            CLRSTAT(uistat_l2append);
                        }else{
                            CLRSTAT(uistat_l1append);
                        }
                        dispstr = ("Append Entry Failed.");
                    }
                }
                break;
                case 12:
                {
                    RemoveDataReply r = qDisassembleRemoveDataReply(msg->payload);
                    if(!(r.errNo)){
                        if(CHKSTAT(uistat_level3)){
                            CLRSTAT(uistat_level3);
                            qList_foreach(currl2e->ld,iter){
                                Level3Entry *ref = (Level3Entry*)iter->data;
                                if(ref->pe.entryid == currl3e->pe.entryid){
                                    qList_erase_elem(currl2e->ld,iter);
                                    break;
                                }
                            }
                            currl3e = NULL;
                        }else if(CHKSTAT(uistat_level2)){
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
                        }else if(CHKSTAT(uistat_level1)){
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
                        dispstr = ("Remove Entry failed.");
                    }
                }
                break;
                case 13:
                {
                    AlterDataReply r = qDisassembleAlterDataReply(msg->payload);
                    if(!(r.errNo)){
                        if(CHKSTAT(uistat_level3)){
                            memcpy(&(currl3e->data),&(l3buffer.data),sizeof(Level3));
                        }else if(CHKSTAT(uistat_level2)){
                            memcpy(&(currl2e->data),&(l2buffer.data),sizeof(Level2));
                        }else if(CHKSTAT(uistat_level1)){
                            memcpy(&(currl1e->data),&(l1buffer.data),sizeof(Level1));
                        }
                    }else{
                        dispstr = ("Alter Entry failed.");
                    }
                }
                break;
                case 20:
                {
                    AlterEntryOwnerReply r = qDisassembleAlterEntryOwnerReply(msg->payload);
                    CLRSTAT(uistat_alterowner);
                    if(!(r.errNo)){
                        if(CHKSTAT(uistat_level3)){
                            currl3e->pe.ownerid = destuid;
                        }else if(CHKSTAT(uistat_level2)){
                            currl2e->pe.ownerid = destuid;
                        }else if(CHKSTAT(uistat_level1)){
                            currl1e->pe.ownerid = destuid;
                        }
                    }else{
                        dispstr = ("Alter Entry Owner failed.");
                    }
                }
                break;
                case 21:
                {
                    AlterEntryGroupReply r = qDisassembleAlterEntryGroupReply(msg->payload);
                    CLRSTAT(uistat_altergroup);
                    if(!(r.errNo)){
                        if(CHKSTAT(uistat_level3)){
                            currl3e->pe.groupid = destgid;
                        }else if(CHKSTAT(uistat_level2)){
                            currl2e->pe.groupid = destgid;
                        }else if(CHKSTAT(uistat_level1)){
                            currl1e->pe.groupid = destgid;
                        }
                    }else{
                        dispstr = ("Alter Entry Group failed.");
                    }
                }
                break;
                case 22:
                {
                    AlterEntryPermissionReply r = qDisassembleAlterEntryPermissionReply(msg->payload);
                    CLRSTAT(uistat_alterowner);
                    if(!(r.errNo)){
                        if(CHKSTAT(uistat_level3)){
                            memcpy(currl3e->pe.permission,destperm,3*sizeof(unsigned char));
                        }else if(CHKSTAT(uistat_level2)){
                            memcpy(currl2e->pe.permission,destperm,3*sizeof(unsigned char));
                        }else if(CHKSTAT(uistat_level1)){
                            memcpy(currl1e->pe.permission,destperm,3*sizeof(unsigned char));
                        }
                    }else{
                        dispstr = ("Alter Entry Owner failed.");
                    }
                }
                break;
                default:
                {
                    dispstr = ("Program control flow reached unexpected part.");
                }
                break;
            }
            qbss_destructor(msg->payload);
            qList_pop_front(ui_notifier);
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
        ImGui::Begin("Current Status##currstat");
        if(dispstr != NULL){
            ImGui::Text("%s",dispstr);
        }
        ImGui::End();
        if(CHKSTAT(uistat_login)){
            ImGui::Begin("Login or Register##login_ui");
            ImGui::Text("Server format: 127.0.0.1:1992");
            ImGui::InputText("Server",serverbuffer,256);
            ImGui::InputText("Username",usernamebuffer,256);
            ImGui::InputText("Password",passwordbuffer,256,ImGuiInputTextFlags_Password);
            if(ImGui::Button("Login")){
                fprintf(stderr,"Attempt login with %s + %s to %s\n",usernamebuffer,passwordbuffer,serverbuffer);
                // TODO:send request to network notifier
                Messeage tmpmsg;
                LoginAlter la;
                // generate tmp bss
                binary_safe_string userbss=qbss_new(),passbss=qbss_new();
                qbss_append(userbss,usernamebuffer,strlen(usernamebuffer));
                qbss_append(passbss,passwordbuffer,strlen(passwordbuffer));
                memcpy(la.srv,serverbuffer,256);
                la.querycont = qAssembleLoginQuery(0,userbss,passbss);
                tmpmsg.payload = qbss_new();
                qbss_append(tmpmsg.payload,(char*)&la,sizeof(la));
                tmpmsg.qid = 0;
                LOCKNET;
                qList_push_back(network_notifier,tmpmsg);
                UNLOCKNET;
                qbss_destructor(userbss);
                qbss_destructor(passbss);
                CLRSTAT(uistat_login);
                SETSTAT(uistat_process_login);
            }
            ImGui::SameLine();
            if(ImGui::Button("Register")){
                fprintf(stderr,"Attempt register with %s + %s to %s\n",usernamebuffer,passwordbuffer,serverbuffer);
                // TODO:send request to network handle
                Messeage tmpmsg;
                LoginAlter la;
                // generate tmp bss
                binary_safe_string userbss=qbss_new(),passbss=qbss_new();
                qbss_append(userbss,usernamebuffer,strlen(usernamebuffer));
                qbss_append(passbss,passwordbuffer,strlen(passwordbuffer));
                memcpy(la.srv,serverbuffer,256);
                la.querycont = qAssembleLoginQuery(1,userbss,passbss);
                tmpmsg.payload = qbss_new();
                qbss_append(tmpmsg.payload,(char*)&la,sizeof(la));
                tmpmsg.qid = 0;
                LOCKNET;
                qList_push_back(network_notifier,tmpmsg);
                UNLOCKNET;
                qbss_destructor(userbss);
                qbss_destructor(passbss);
                CLRSTAT(uistat_login);
                SETSTAT(uistat_process_login);
            }
            ImGui::End();
        }
        if(CHKSTAT(uistat_process_login)){
            ImGui::Begin("Please Wait##process_login_ui");
            ImGui::Text("Connecting to server..");
            ImGui::End();
        }
        if(CHKSTAT(uistat_syncdata)){
            ImGui::Begin("Please Wait##sync_data_ui");
            ImGui::Text("Synchronizing data with server..");
            ImGui::End();
        }
        if(CHKDIRT(uistat_syncdata)){
            CLRDIRT(uistat_syncdata);
            Messeage tmpmsg;
            tmpmsg.qid = 1;
            tmpmsg.payload = qAssembleSyncDataQuery(uid,gid);
            LOCKNET;
            qList_push_back(network_notifier,tmpmsg);
            UNLOCKNET;
        }
        if(CHKSTAT(uistat_main)){
            ImGui::Begin("ManagerMain -- CarTypes");
            ImGui::Text("Current User: %s",usernamebuffer);
            ImGui::SameLine();
            if(ImGui::Button("Alter my password")){
                SETSTAT(uistat_alterpass);
            }
            ImGui::SameLine();
            if(ImGui::Button("Administrator Panel")){
                if(uid == 0)
                    SETSTAT(uistat_admin);
                else{
                    dispstr = ("Permission Denied.");
                }
            }
            if(ImGui::Button("Append")){
                SETSTAT(uistat_l1append);
            }
            ImGui::SameLine();
            if(ImGui::Button("Lookup##mainw")){
                SETSTAT(uistat_lookup);
            }
            ImGui::SameLine();
            if(ImGui::Button("Statistics##mainw")){
                SETSTAT(uistat_statistic);
            }
            if(ImGui::Button("Show detail of current selection")){
                if(currl1e != NULL){
                    SETSTAT(uistat_level1);
                }else{
                    dispstr = ("Selected entry is invalid.");
                }
            }
            ImGui::Columns(8,"CarTypes");
            ImGui::Separator();
            GUICOLUMNNEXT("UUID");
            GUICOLUMNNEXT("CarId");
            GUICOLUMNNEXT("CarName");
            GUICOLUMNNEXT("Weight");
            GUICOLUMNNEXT("SeatNumber");
            GUICOLUMNNEXT("Length");
            GUICOLUMNNEXT("Width");
            GUICOLUMNNEXT("Height");
            ImGui::Separator();
            qList_foreach(*data,iter){
                Level1Entry *le = (Level1Entry*)iter->data;
                char tmpaddr[16];
                memset(tmpaddr,0,16);
                sprintf(tmpaddr,"%p",le);
                if(ImGui::Selectable(tmpaddr,currl1e == le, ImGuiSelectableFlags_SpanAllColumns)){
                    if(CHKSTAT(uistat_level1)){
                        dispstr = ("You cannot select another while editing one.");
                    }else{
                        currl1e = le;
                        memcpy(&(l1buffer.data),&(le->data),sizeof(Level1));
                    }
                }
                ImGui::NextColumn();
                GUICOLUMNNEXT("%s",le->data.carId);
                GUICOLUMNNEXT("%s",le->data.carName);
                GUICOLUMNNEXT("%d",le->data.weight);
                GUICOLUMNNEXT("%d",le->data.seatNum);
                GUICOLUMNNEXT("%d",le->data.length);
                GUICOLUMNNEXT("%d",le->data.width);
                GUICOLUMNNEXT("%d",le->data.height);
            }
            ImGui::Columns(1);
            ImGui::End();
        }
        if(CHKSTAT(uistat_alterpass)){
            ImGui::Begin("Alter Password");
            ImGui::InputText("Original Password",origpassbuffer,256,ImGuiInputTextFlags_Password);
            ImGui::InputText("New Password",alterpassbuffer,256,ImGuiInputTextFlags_Password);
            if(ImGui::Button("Confirm Alternation")){
                if(fullstrcmp(passwordbuffer,origpassbuffer)){
                    Messeage altmsg;
                    altmsg.qid = 1;
                    binary_safe_string tmpbss = qbss_new();
                    qbss_append(tmpbss,alterpassbuffer,strlen(alterpassbuffer));
                    altmsg.payload = qAssembleAlterPassQuery(uid,tmpbss.size,tmpbss);
                    LOCKNET;
                    qList_push_back(network_notifier,altmsg);
                    UNLOCKNET;
                    qbss_destructor(tmpbss);
                }else{
                    CLRSTAT(uistat_alterpass);
                    dispstr = ("Original password is incorrect.");
                }
            }
            ImGui::SameLine();
            if(ImGui::Button("Cancel##alter_pass")){
                CLRSTAT(uistat_alterpass);
            }
            ImGui::End();
        }
        if(CHKSTAT(uistat_level1)){
            ImGui::Begin("ManagerInterface -- SellInfos");
            ImGui::InputText("CarId",l1buffer.data.carId,20);
            ImGui::InputText("CarName",l1buffer.data.carName,20);
            ImGui::InputInt("Weight",&(l1buffer.data.weight));
            ImGui::InputInt("SeatNum",&(l1buffer.data.seatNum));
            ImGui::InputInt("Length",&(l1buffer.data.length));
            ImGui::InputInt("Width",&(l1buffer.data.width));
            ImGui::InputInt("Height",&(l1buffer.data.height));
            if(ImGui::Button("Alter##level1_detail")){
                Messeage m;
                m.qid = 1;
                binary_safe_string tmpbss = qbss_new();
                qbss_append(tmpbss,(char*)&(l1buffer.data),sizeof(l1buffer.data));
                ui entids[3];
                entids[0]=currl1e->pe.entryid;
                m.payload = qAssembleAlterDataQuery(uid,gid,0,entids,tmpbss);
                LOCKNET;
                qList_push_back(network_notifier,m);
                UNLOCKNET;
                qbss_destructor(tmpbss);
            }
            ImGui::SameLine();
            if(ImGui::Button("Show selected details##level1_detail")){
                if(currl2e != NULL){
                    SETSTAT(uistat_level2);
                }else{
                    dispstr = ("Selected entry is invalid.");
                }
            }
            ImGui::SameLine();
            if(ImGui::Button("Append##level1_detail")){
                SETSTAT(uistat_l2append);
            }
            if(ImGui::Button("AlterOwner##level1_detail")){
                SETSTAT(uistat_alterowner);
            }
            ImGui::SameLine();
            if(ImGui::Button("AlterGroup##level1_detail")){
                SETSTAT(uistat_altergroup);
            }
            ImGui::SameLine();
            if(ImGui::Button("AlterPermission##Level1_detail")){
                SETSTAT(uistat_alterperm);
            }
            if(ImGui::Button("Remove##level1_detail")){
                Messeage m;
                m.qid = 1;
                ui tmpids[3];
                tmpids[0]=currl1e->pe.entryid;
                m.payload = qAssembleRemoveDataQuery(uid,gid,0,tmpids);
                LOCKNET;
                qList_push_back(network_notifier,m);
                UNLOCKNET;
            }
            ImGui::SameLine();
            if(ImGui::Button("Close##Level1_detail")){
                if(CHKSTAT(uistat_level2)){
                    dispstr = ("This window is required by another.");
                }else{
                    currl2e = NULL;
                    CLRSTAT(uistat_level1);
                }
            }
            ImGui::Separator();
            ImGui::Columns(9,"SellInfos");
            ImGui::Separator();
            GUICOLUMNNEXT("UUID");
            GUICOLUMNNEXT("CarId");
            GUICOLUMNNEXT("CarName");
            GUICOLUMNNEXT("Color");
            GUICOLUMNNEXT("SellDate");
            GUICOLUMNNEXT("CustName");
            GUICOLUMNNEXT("CustId");
            GUICOLUMNNEXT("CustTel");
            GUICOLUMNNEXT("Price");
            ImGui::Separator();
            qList_foreach(currl1e->ld,iter){
                Level2Entry *le = (Level2Entry*)iter->data;
                char tmpaddr[16];
                sprintf(tmpaddr,"%p",le);
                if(ImGui::Selectable(tmpaddr,currl2e == le,ImGuiSelectableFlags_SpanAllColumns)){
                    fprintf(stderr,"detected selection try on %p current %p\n",le,currl2e);
                    if(CHKSTAT(uistat_level2)){
                        dispstr = ("You cannot select another while editing one.");
                    }else{
                        currl2e = le;
                        memcpy(&(l2buffer.data),&(le->data),sizeof(Level2));
                        {
                            unsigned char* tmprefr = (unsigned char*)&(le->data.color);
                            tmprefr ++;
                            tmpcolorR = tmprefr[0];
                            tmpcolorG = tmprefr[1];
                            tmpcolorB = tmprefr[2];
                        }
                    }
                }
                ImGui::NextColumn();
                GUICOLUMNNEXT("%s",le->data.carId);
                GUICOLUMNNEXT("%s",le->data.carName);
                //GUICOLUMNNEXT("%u-%u-%u",(unsigned int)(*(((unsigned char*)&(le->data.color))+1)),
                //(unsigned int)(*(((unsigned char*)&(le->data.color))+2)),(unsigned int)(*(((unsigned char*)&(le->data.color))+3)));
                {
                    unsigned char* tmprefr = (unsigned char*)&(le->data.color);
                    tmprefr ++;
                    ImGui::ColorButton("Color##l1details",ImVec4(tmprefr[0]/255.0f,tmprefr[1]/255.0f,tmprefr[2]/255.0f,0));
                    ImGui::NextColumn();
                }
                GUICOLUMNNEXT("%s",le->data.selldate);
                GUICOLUMNNEXT("%s",le->data.customerName);
                GUICOLUMNNEXT("%s",le->data.customerId);
                GUICOLUMNNEXT("%s",le->data.customerTel);
                GUICOLUMNNEXT("%.2f",le->data.priceSum);
            }
            ImGui::Columns(1);
            ImGui::End();
        }
        if(CHKSTAT(uistat_level2)){
            ImGui::Begin("ManagerInterface -- PaymentRecords");
            ImGui::InputText("CarId",l2buffer.data.carId,20);
            ImGui::InputText("CarName",l2buffer.data.carName,20);
            ImGui::InputInt("Color R",&tmpcolorR);
            ImGui::InputInt("Color G",&tmpcolorG);
            ImGui::InputInt("Color B",&tmpcolorB);
            ImGui::ColorButton("Color",ImVec4(tmpcolorR/255.0f,tmpcolorG/255.0f,tmpcolorB/255.0f,0));
            ImGui::InputText("SellDate",l2buffer.data.selldate,12);
            ImGui::InputText("CustName",l2buffer.data.customerName,20);
            ImGui::InputText("CustId",l2buffer.data.customerId,18);
            ImGui::InputText("CustTel",l2buffer.data.customerTel,20);
            ImGui::InputFloat("Price",&(l2buffer.data.priceSum));
            if(ImGui::Button("Alter##level2_detail")){
                // setcolor
                {
                    unsigned char* tmpref = (unsigned char*)&(l2buffer.data.color);
                    tmpref++;
                    tmpref[0]=(unsigned char)tmpcolorR;
                    tmpref[1]=(unsigned char)tmpcolorG;
                    tmpref[2]=(unsigned char)tmpcolorB;
                }
                Messeage m;
                m.qid = 1;
                binary_safe_string tmpbss = qbss_new();
                qbss_append(tmpbss,(char*)&(l2buffer.data),sizeof(l2buffer.data));
                ui entids[3];
                entids[0]=currl1e->pe.entryid;
                entids[1]=currl2e->pe.entryid;
                m.payload=qAssembleAlterDataQuery(uid,gid,1,entids,tmpbss);
                LOCKNET;
                qList_push_back(network_notifier,m);
                UNLOCKNET;
                qbss_destructor(tmpbss);
            }
            ImGui::SameLine();
            if(ImGui::Button("Show selected details##level2_detail")){
                if(currl3e != NULL){
                    SETSTAT(uistat_level3);
                }else{
                    dispstr = ("Selected entry is invalid.");
                }
            }
            ImGui::SameLine();
            if(ImGui::Button("Append##level2_detail")){
                SETSTAT(uistat_l3append);
            }
            if(ImGui::Button("AlterOwner##level2_detail")){
                SETSTAT(uistat_alterowner);
            }
            ImGui::SameLine();
            if(ImGui::Button("AlterGroup##level2_detail")){
                SETSTAT(uistat_altergroup);
            }
            ImGui::SameLine();
            if(ImGui::Button("AlterPermission##level2_detail")){
                SETSTAT(uistat_alterperm);
            }
            if(ImGui::Button("Remove##level2_detail")){
                Messeage m;
                m.qid = 1;
                ui tmpids[3];
                tmpids[0]=currl1e->pe.entryid;
                tmpids[1]=currl2e->pe.entryid;
                m.payload = qAssembleRemoveDataQuery(uid,gid,1,tmpids);
                LOCKNET;
                qList_push_back(network_notifier,m);
                UNLOCKNET;
            }
            ImGui::SameLine();
            if(ImGui::Button("Close##Level2_detail")){
                if(CHKSTAT(uistat_level3)){
                    dispstr = ("This window is required by another.");
                }else{
                    currl3e = NULL;
                    CLRSTAT(uistat_level2);
                }
            }
            ImGui::Separator();
            ImGui::Columns(6,"PayRecords");
            ImGui::Separator();
            GUICOLUMNNEXT("UUID");
            GUICOLUMNNEXT("CarId");
            GUICOLUMNNEXT("Paydate");
            GUICOLUMNNEXT("Amount");
            GUICOLUMNNEXT("Remain");
            GUICOLUMNNEXT("Seller");
            ImGui::Separator();
            qList_foreach(currl2e->ld,iter){
                Level3Entry *le = (Level3Entry*)iter->data;
                char tmpaddr[16];
                sprintf(tmpaddr,"%p",le);
                if(ImGui::Selectable(tmpaddr,currl3e==le,ImGuiSelectableFlags_SpanAllColumns)){
                    if(CHKSTAT(uistat_level3)){
                        dispstr = ("You cannot select another entry while editing one.");
                    }else{
                        currl3e=le;
                        memcpy(&(l3buffer.data),&(le->data),sizeof(Level3));
                    }
                }
                ImGui::NextColumn();
                GUICOLUMNNEXT("%s",le->data.carId);
                GUICOLUMNNEXT("%s",le->data.paydate);
                GUICOLUMNNEXT("%.2f",le->data.amount);
                GUICOLUMNNEXT("%.2f",le->data.remain);
                GUICOLUMNNEXT("%s",le->data.sellerId);
            }
            ImGui::Columns(1);
            ImGui::End();
        }
        if(CHKSTAT(uistat_level3)){
            ImGui::Begin("Manager Window -- Payment Record Details");
            ImGui::InputText("CarId",l3buffer.data.carId,20);
            ImGui::InputText("PayDate",l3buffer.data.paydate,12);
            ImGui::InputFloat("Amount",&(l3buffer.data.amount));
            ImGui::InputFloat("Remains",&(l3buffer.data.remain));
            ImGui::InputText("Seller",l3buffer.data.sellerId,20);
            if(ImGui::Button("Alter##level3_detail")){
                Messeage m;
                m.qid = 1;
                binary_safe_string tmpbss = qbss_new();
                qbss_append(tmpbss,(char*)&(l3buffer.data),sizeof(l3buffer.data));
                ui entids[3];
                entids[0]=currl1e->pe.entryid;
                entids[1]=currl2e->pe.entryid;
                entids[2]=currl3e->pe.entryid;
                m.payload = qAssembleAlterDataQuery(uid,gid,2,entids,tmpbss);
                LOCKNET;
                qList_push_back(network_notifier,m);
                UNLOCKNET;
                qbss_destructor(tmpbss);
            }
            ImGui::SameLine();
            if(ImGui::Button("AlterOwner##level3_detail")){
                SETSTAT(uistat_alterowner);
            }
            if(ImGui::Button("AlterGroup##level3_detail")){
                SETSTAT(uistat_altergroup);
            }
            ImGui::SameLine();
            if(ImGui::Button("AlterPermission##level3_detail")){
                SETSTAT(uistat_alterperm);
            }
            if(ImGui::Button("Remove##level3_detail")){
                Messeage m;
                m.qid = 1;
                ui tmpids[3];
                tmpids[0] = currl1e->pe.entryid;
                tmpids[1] = currl2e->pe.entryid;
                tmpids[2] = currl3e->pe.entryid;
                m.payload = qAssembleRemoveDataQuery(uid,gid,2,tmpids);
                LOCKNET;
                qList_push_back(network_notifier,m);
                UNLOCKNET;
            }
            ImGui::SameLine();
            if(ImGui::Button("Close##level3_detail")){
                currl3e = NULL;
                CLRSTAT(uistat_level3);
            }
            ImGui::End();
        }
        if(CHKSTAT(uistat_admin)){
            ImGui::Begin("Administrator Panel##w");
            if(ImGui::Button("List Users##admin_panel")){
                SETSTAT(uistat_syncuser);
                SETDIRT(uistat_syncuser);
            }
            ImGui::SameLine();
            if(ImGui::Button("List Groups##admin_panel")){
                SETSTAT(uistat_syncgroup);
                SETDIRT(uistat_syncgroup);
            }
            if(ImGui::Button("Move User to Group##admin_panel")){
                SETSTAT(uistat_setgroup);
            }
            ImGui::InputText("Ultimate Password",shutpassbuffer,256);
            if(ImGui::Button("Shutdown Server##admin_panel")){
                Messeage m;
                m.qid = 1;
                binary_safe_string tmpbss = qbss_new();
                qbss_append(tmpbss,shutpassbuffer,strlen(shutpassbuffer));
                m.payload = qAssembleStopServerQuery(tmpbss);
                LOCKNET;
                qList_push_back(network_notifier,m);
                UNLOCKNET;
                qbss_destructor(tmpbss);
                CLRSTAT(uistat_admin);
                Messeage closem;
                m.qid = 255;
                LOCKNET;
                qList_push_back(network_notifier,closem);
                UNLOCKNET;
            }
            if(ImGui::Button("Close##admin_panel")){
                CLRSTAT(uistat_admin);
            }
            ImGui::End();
        }
        if(CHKSTAT(uistat_setgroup)){
            ImGui::Begin("Group Alternation##setgrp");
            ImGui::InputInt("Target User Id",(int*)&destuid);
            ImGui::InputInt("Target Group Id",(int*)&destgid);
            if(ImGui::Button("Confirm Alternation##setgrp")){
                Messeage m;
                m.qid = 1;
                m.payload = qAssembleAlterGroupQuery(uid,destuid,destgid);
                LOCKNET;
                qList_push_back(network_notifier,m);
                UNLOCKNET;
            }
            ImGui::SameLine();
            if(ImGui::Button("Cancel##setgrp")){
                CLRSTAT(uistat_setgroup);
            }
            ImGui::End();
        }
        if(CHKSTAT(uistat_syncgroup)){
            if(CHKDIRT(uistat_syncgroup)){
                CLRDIRT(uistat_syncgroup);
                Messeage m;
                m.qid = 1;
                m.payload = qAssembleListGroupQuery(uid);
                LOCKNET;
                qList_push_back(network_notifier,m);
                UNLOCKNET;
            }
            ImGui::Begin("Please Wait##sync_group");
            ImGui::Text("Synchronizing group data with server..");
            ImGui::End();
        }
        if(CHKSTAT(uistat_showgroup)){
            ImGui::Begin("AdminInterface -- Groups");
            if(ImGui::Button("Close##admin_groups")){
                CLRSTAT(uistat_showgroup);
            }
            ImGui::InputInt("group id",(int*)&destrmgid);
            if(ImGui::Button("Remove")){
                Messeage m;
                m.qid = 1;
                m.payload = qAssembleRemoveGroupQuery(uid,destrmgid);
                LOCKNET;
                qList_push_back(network_notifier,m);
                UNLOCKNET;
            }
            ImGui::Text("Avaliable Groups(Only gid)");
            ImGui::Separator();
            qList_foreach(*grouplist,iter){
                GroupData *g = (GroupData*)iter->data;
                ImGui::Text("%d",g->gid);
                ImGui::Separator();
            }
            ImGui::End();
        }
        if(CHKSTAT(uistat_syncuser)){
            if(CHKDIRT(uistat_syncuser)){
                CLRDIRT(uistat_syncuser);
                Messeage m;
                m.qid = 1;
                m.payload = qAssembleListUserQuery(uid);
                LOCKNET;
                qList_push_back(network_notifier,m);
                UNLOCKNET;
            }
            ImGui::Begin("Please Wait##sync_user");
            ImGui::Text("Synchronizing user data with server..");
            ImGui::End();
        }
        if(CHKSTAT(uistat_showuser)){
            ImGui::Begin("AdminInterfase -- Users");
            if(ImGui::Button("Close##admin_users")){
                CLRSTAT(uistat_showuser);
            }
            ImGui::InputInt("user id",(int*)&destrmuid);
            if(ImGui::Button("Remove")){
                Messeage m;
                m.qid = 1;
                m.payload = qAssembleRemoveUserQuery(uid,destrmuid);
                LOCKNET;
                qList_push_back(network_notifier,m);
                UNLOCKNET;
            }
            ImGui::Separator();
            ImGui::Columns(3,"Uses");
            ImGui::Separator();
            GUICOLUMNNEXT("UserId");
            GUICOLUMNNEXT("GroupId");
            GUICOLUMNNEXT("Username");
            ImGui::Separator();
            qList_foreach(*userlist,iter){
                UserData* u = (UserData*)iter->data;
                GUICOLUMNNEXT("%u",u->uid);
                GUICOLUMNNEXT("%u",u->gid);
                GUICOLUMNNEXT("%s",u->username);
            }
            ImGui::Columns(1);
            ImGui::End();
        }
        if(CHKSTAT(uistat_l1append)){
            ImGui::Begin("Append Data##l1append");
            ImGui::InputText("CarId",l1buffer.data.carId,20);
            ImGui::InputText("CarName",l1buffer.data.carName,20);
            ImGui::InputInt("Weight",&(l1buffer.data.weight));
            ImGui::InputInt("SeatNum",&(l1buffer.data.seatNum));
            ImGui::InputInt("Length",&(l1buffer.data.length));
            ImGui::InputInt("Width",&(l1buffer.data.width));
            ImGui::InputInt("Height",&(l1buffer.data.height));
            if(ImGui::Button("Append##level1_append")){
                Messeage m;
                m.qid = 1;
                binary_safe_string tmpbss = qbss_new();
                qbss_append(tmpbss,(char*)&(l1buffer.data),sizeof(l1buffer.data));
                ui entids[3];
                m.payload = qAssembleAppendDataQuery(uid,gid,0,entids,tmpbss);
                LOCKNET;
                qList_push_back(network_notifier,m);
                UNLOCKNET;
                qbss_destructor(tmpbss);
            }
            ImGui::SameLine();
            if(ImGui::Button("Close##level1_append")){
                CLRSTAT(uistat_l1append);
            }
            ImGui::End();
        }
        if(CHKSTAT(uistat_l2append)){
            ImGui::Begin("Append Data##l2append");
            ImGui::InputText("CarId",l2buffer.data.carId,20);
            ImGui::InputText("CarName",l2buffer.data.carName,20);
            ImGui::InputInt("Color R",&tmpcolorR);
            ImGui::InputInt("Color G",&tmpcolorG);
            ImGui::InputInt("Color B",&tmpcolorB);
            ImGui::ColorButton("Color",ImVec4(tmpcolorR/255.0f,tmpcolorG/255.0f,tmpcolorB/255.0f,0));
            ImGui::InputText("SellDate",l2buffer.data.selldate,12);
            ImGui::InputText("CustName",l2buffer.data.customerName,20);
            ImGui::InputText("CustId",l2buffer.data.customerId,18);
            ImGui::InputText("CustTel",l2buffer.data.customerTel,20);
            ImGui::InputFloat("Price",&(l2buffer.data.priceSum));
            if(ImGui::Button("Append##level2_append")){
                fprintf(stderr,"Append to %u --\n",currl1e->pe.entryid);
                {
                    unsigned char* tmpref = (unsigned char*)&(l2buffer.data.color);
                    tmpref++;
                    tmpref[0]=(unsigned char)tmpcolorR;
                    tmpref[1]=(unsigned char)tmpcolorG;
                    tmpref[2]=(unsigned char)tmpcolorB;
                }
                Messeage m;
                m.qid = 1;
                binary_safe_string tmpbss = qbss_new();
                qbss_append(tmpbss,(char*)&(l2buffer.data),sizeof(l2buffer.data));
                ui entids[3];
                entids[0] = currl1e->pe.entryid;
                m.payload = qAssembleAppendDataQuery(uid,gid,1,entids,tmpbss);
                LOCKNET;
                qList_push_back(network_notifier,m);
                UNLOCKNET;
                qbss_destructor(tmpbss);
            }
            ImGui::SameLine();
            if(ImGui::Button("Close##level2_append")){
                CLRSTAT(uistat_l2append);
            }
            ImGui::End();
        }
        if(CHKSTAT(uistat_l3append)){
            ImGui::Begin("Append Data##l3append");
            ImGui::InputText("CarId",l3buffer.data.carId,20);
            ImGui::InputText("PayDate",l3buffer.data.paydate,12);
            ImGui::InputFloat("Amount",&(l3buffer.data.amount));
            ImGui::InputFloat("Remains",&(l3buffer.data.remain));
            ImGui::InputText("Seller",l3buffer.data.sellerId,20);
            if(ImGui::Button("Append##level3_append")){
                Messeage m;
                m.qid = 1;
                binary_safe_string tmpbss = qbss_new();
                qbss_append(tmpbss,(char*)&(l3buffer.data),sizeof(l3buffer.data));
                ui entids[3];
                entids[0]=currl1e->pe.entryid;
                entids[1]=currl2e->pe.entryid;
                m.payload = qAssembleAppendDataQuery(uid,gid,2,entids,tmpbss);
                LOCKNET;
                qList_push_back(network_notifier,m);
                UNLOCKNET;
                qbss_destructor(tmpbss);
            }
            ImGui::SameLine();
            if(ImGui::Button("Close##level3_append")){
                CLRSTAT(uistat_l3append);
            }
            ImGui::End();
        }
        if(CHKSTAT(uistat_alterowner)){
            ImGui::Begin("Alter Owner of Entry");
            ImGui::InputInt("Destintation Uid",(int*)&destuid);
            if(ImGui::Button("Alter##alter_owner")){
                Messeage m;
                m.qid = 1;
                ui entids[3];
                ui levels=0;
                if(CHKSTAT(uistat_level1)){
                    entids[0]=currl1e->pe.entryid;
                    levels=0;
                }
                if(CHKSTAT(uistat_level2)){
                    entids[1]=currl2e->pe.entryid;
                    levels=1;
                }
                if(CHKSTAT(uistat_level3)){
                    entids[2]=currl3e->pe.entryid;
                    levels = 2;
                }
                m.payload = qAssembleAlterEntryOwnerQuery(uid,gid,destuid,levels,entids);
                LOCKNET;
                qList_push_back(network_notifier,m);
                UNLOCKNET;
            }
            if(ImGui::Button("Close##alter_owner")){
                CLRSTAT(uistat_alterowner);
            }
            ImGui::End();
        }
        if(CHKSTAT(uistat_altergroup)){
            ImGui::Begin("Alter Group of Entry");
            ImGui::InputInt("Destintation Gid",(int*)&destgid);
            if(ImGui::Button("Alter##alter_group")){
                Messeage m;
                m.qid = 1;
                ui entids[3];
                ui levels=0;
                if(CHKSTAT(uistat_level1)){
                    entids[0]=currl1e->pe.entryid;
                    levels=0;
                }
                if(CHKSTAT(uistat_level2)){
                    entids[1]=currl2e->pe.entryid;
                    levels=1;
                }
                if(CHKSTAT(uistat_level3)){
                    entids[2]=currl3e->pe.entryid;
                    levels = 2;
                }
                m.payload = qAssembleAlterEntryGroupQuery(uid,gid,destgid,levels,entids);
                LOCKNET;
                qList_push_back(network_notifier,m);
                UNLOCKNET;
            }
            if(ImGui::Button("Close##alter_group")){
                CLRSTAT(uistat_altergroup);
            }
            ImGui::End();
        }
        if(CHKSTAT(uistat_alterperm)){
            ImGui::Begin("Alter Permission of Entry");
            ImGui::InputText("Permission Mask",alterpermbuffer,10);
            if(ImGui::Button("Alter##alter_perm")){
                Messeage m;
                m.qid = 1;
                ui entids[3];
                ui levels=0;
                if(CHKSTAT(uistat_level1)){
                    entids[0]=currl1e->pe.entryid;
                    levels=0;
                }
                if(CHKSTAT(uistat_level2)){
                    entids[1]=currl2e->pe.entryid;
                    levels=1;
                }
                if(CHKSTAT(uistat_level3)){
                    entids[2]=currl3e->pe.entryid;
                    levels = 2;
                }
                memset(destperm,0,sizeof(unsigned char)*3);
                if(alterpermbuffer[0]!='-'){
                    destperm[0] |= Q_PERMISSION_C;
                }
                if(alterpermbuffer[1]!='-'){
                    destperm[0] |= Q_PERMISSION_R;
                }
                if(alterpermbuffer[2]!='-'){
                    destperm[0] |= Q_PERMISSION_W;
                }
                if(alterpermbuffer[3]!='-'){
                    destperm[1] |= Q_PERMISSION_C;
                }
                if(alterpermbuffer[4]!='-'){
                    destperm[1] |= Q_PERMISSION_R;
                }
                if(alterpermbuffer[5]!='-'){
                    destperm[1] |= Q_PERMISSION_W;
                }
                if(alterpermbuffer[6]!='-'){
                    destperm[2] |= Q_PERMISSION_C;
                }
                if(alterpermbuffer[7]!='-'){
                    destperm[2] |= Q_PERMISSION_R;
                }
                if(alterpermbuffer[8]!='-'){
                    destperm[2] |= Q_PERMISSION_W;
                }
                m.payload = qAssembleAlterEntryPermissionQuery(uid,gid,destperm,levels,entids);
                LOCKNET;
                qList_push_back(network_notifier,m);
                UNLOCKNET;
            }
            if(ImGui::Button("Close##alter_perm")){
                CLRSTAT(uistat_alterperm);
            }
            ImGui::End();
        }
        if(CHKSTAT(uistat_lookup)){
            ImGui::Begin("Lookup Entry##lookup_entry");
            ImGui::InputText("Keyword",lookupbuffer,256);
            if(ImGui::Button("Search CarId##lookup")){
                if(!CHKSTAT(uistat_level1)){
                    lookupstat = 1;
                }else{
                    dispstr = "You cannot search while editing.";
                }
            }
            ImGui::SameLine();
            if(ImGui::Button("Search CustName##lookup")){
                if(!CHKSTAT(uistat_level1)){
                    lookupstat = 2;
                }else{
                    dispstr = "You cannot search while editing.";
                }
            }
            ImGui::SameLine();
            if(ImGui::Button("Search Seller##lookup")){
                if(!CHKSTAT(uistat_level1)){
                    lookupstat = 3;
                }else{
                    dispstr = "You cannot search while editing.";
                }
            }
            if(ImGui::Button("Show Details##lookup")){
                if(lookupstat >= 1){
                    SETSTAT(uistat_level1);
                }
                if(lookupstat >= 2){
                    SETSTAT(uistat_level2);
                }
                if(lookupstat >= 3){
                    SETSTAT(uistat_level3);
                }
            }
            ImGui::SameLine();
            if(ImGui::Button("Clear##lookup")){
                lookupstat = 0;
            }
            if(CHKSTAT(uistat_level1)){
                lookupstat = 0;
            }
            ImGui::SameLine();
            if(ImGui::Button("Close##lookup")){
                CLRSTAT(uistat_lookup);
            }
            if(lookupstat == 1){
                ImGui::Separator();
                ImGui::Columns(8,"CarTypes");
                ImGui::Separator();
                GUICOLUMNNEXT("UUID");
                GUICOLUMNNEXT("CarId");
                GUICOLUMNNEXT("CarName");
                GUICOLUMNNEXT("Weight");
                GUICOLUMNNEXT("SeatNumber");
                GUICOLUMNNEXT("Length");
                GUICOLUMNNEXT("Width");
                GUICOLUMNNEXT("Height");
                ImGui::Separator();
                qList_foreach(*data,iter){
                    Level1Entry *le = (Level1Entry*)iter->data;
                    if(partstrcmp(le->data.carId,lookupbuffer)){
                        char tmpaddr[16];
                        memset(tmpaddr,0,16);
                        sprintf(tmpaddr,"%p",le);
                        if(ImGui::Selectable(tmpaddr,currl1e == le, ImGuiSelectableFlags_SpanAllColumns)){
                            if(CHKSTAT(uistat_level1)){
                                dispstr = ("You cannot select another while editing one.");
                            }else{
                                currl1e = le;
                                memcpy(&(l1buffer.data),&(le->data),sizeof(Level1));
                            }
                        }
                        ImGui::NextColumn();
                        GUICOLUMNNEXT("%s",le->data.carId);
                        GUICOLUMNNEXT("%s",le->data.carName);
                        GUICOLUMNNEXT("%d",le->data.weight);
                        GUICOLUMNNEXT("%d",le->data.seatNum);
                        GUICOLUMNNEXT("%d",le->data.length);
                        GUICOLUMNNEXT("%d",le->data.width);
                        GUICOLUMNNEXT("%d",le->data.height);
                    }
                }
                ImGui::Columns(1);
            }else if(lookupstat == 2){
                ImGui::Separator();
                ImGui::Columns(9,"SellInfos");
                ImGui::Separator();
                GUICOLUMNNEXT("UUID");
                GUICOLUMNNEXT("CarId");
                GUICOLUMNNEXT("CarName");
                GUICOLUMNNEXT("Color");
                GUICOLUMNNEXT("SellDate");
                GUICOLUMNNEXT("CustName");
                GUICOLUMNNEXT("CustId");
                GUICOLUMNNEXT("CustTel");
                GUICOLUMNNEXT("Price");
                ImGui::Separator();
                qList_foreach(*data,iter){
                    Level1Entry *tmple = (Level1Entry*)iter->data;
                    if(tmple->ld.size != 0){
                        qList_foreach(tmple->ld,iiter){
                            Level2Entry *le = (Level2Entry*)iiter->data;
                            if(partstrcmp(le->data.customerName,lookupbuffer)){
                                char tmpaddr[16];
                                sprintf(tmpaddr,"%p",le);
                                if(ImGui::Selectable(tmpaddr,currl2e == le,ImGuiSelectableFlags_SpanAllColumns)){
                                    fprintf(stderr,"detected selection try on %p current %p\n",le,currl2e);
                                    if(CHKSTAT(uistat_level2)){
                                        dispstr = ("You cannot select another while editing one.");
                                    }else{
                                        currl1e = tmple;
                                        currl2e = le;
                                        memcpy(&(l2buffer.data),&(le->data),sizeof(Level2));
                                        {
                                            unsigned char* tmprefr = (unsigned char*)&(le->data.color);
                                            tmprefr ++;
                                            tmpcolorR = tmprefr[0];
                                            tmpcolorG = tmprefr[1];
                                            tmpcolorB = tmprefr[2];
                                        }
                                    }
                                }
                                ImGui::NextColumn();
                                GUICOLUMNNEXT("%s",le->data.carId);
                                GUICOLUMNNEXT("%s",le->data.carName);
                                //GUICOLUMNNEXT("%u-%u-%u",(unsigned int)(*(((unsigned char*)&(le->data.color))+1)),
                                //(unsigned int)(*(((unsigned char*)&(le->data.color))+2)),(unsigned int)(*(((unsigned char*)&(le->data.color))+3)));
                                {
                                    unsigned char* tmprefr = (unsigned char*)&(le->data.color);
                                    tmprefr ++;
                                    ImGui::ColorButton("Color##l1details",ImVec4(tmprefr[0]/255.0f,tmprefr[1]/255.0f,tmprefr[2]/255.0f,0));
                                    ImGui::NextColumn();
                                }
                                GUICOLUMNNEXT("%s",le->data.selldate);
                                GUICOLUMNNEXT("%s",le->data.customerName);
                                GUICOLUMNNEXT("%s",le->data.customerId);
                                GUICOLUMNNEXT("%s",le->data.customerTel);
                                GUICOLUMNNEXT("%.2f",le->data.priceSum);
                            }
                        }
                    }
                }
                ImGui::Columns(1);
            }else if(lookupstat == 3){
                ImGui::Separator();
                ImGui::Columns(6,"PayRecords");
                ImGui::Separator();
                GUICOLUMNNEXT("UUID");
                GUICOLUMNNEXT("CarId");
                GUICOLUMNNEXT("Paydate");
                GUICOLUMNNEXT("Amount");
                GUICOLUMNNEXT("Remain");
                GUICOLUMNNEXT("Seller");
                ImGui::Separator();
                qList_foreach(*data,iter){
                    Level1Entry *tmple = (Level1Entry*)iter->data;
                    if(tmple->ld.size != 0){
                        qList_foreach(tmple->ld,iiter){
                            Level2Entry *tmplle = (Level2Entry*)iiter->data;
                            if(tmplle->ld.size != 0){
                                qList_foreach(tmplle->ld,iiiter){
                                    Level3Entry *le = (Level3Entry*)iiiter->data;
                                    if(partstrcmp(le->data.sellerId,lookupbuffer)){
                                        char tmpaddr[16];
                                        sprintf(tmpaddr,"%p",le);
                                        if(ImGui::Selectable(tmpaddr,currl3e==le,ImGuiSelectableFlags_SpanAllColumns)){
                                            if(CHKSTAT(uistat_level3)){
                                                dispstr = ("You cannot select another entry while editing one.");
                                            }else{
                                                currl1e = tmple;
                                                currl2e = tmplle;
                                                currl3e=le;
                                                memcpy(&(l3buffer.data),&(le->data),sizeof(Level3));
                                            }
                                        }
                                        ImGui::NextColumn();
                                        GUICOLUMNNEXT("%s",le->data.carId);
                                        GUICOLUMNNEXT("%s",le->data.paydate);
                                        GUICOLUMNNEXT("%.2f",le->data.amount);
                                        GUICOLUMNNEXT("%.2f",le->data.remain);
                                        GUICOLUMNNEXT("%s",le->data.sellerId);
                                    }
                                }
                            }
                        }
                    }
                }
                ImGui::Columns(1);
            }
            ImGui::End();
        }
        if(CHKSTAT(uistat_statistic)){
            ImGui::Begin("Statistia##statistics");
            if(ImGui::Button("Sort by SoldNum##statistics")){
                if(!CHKSTAT(uistat_level1)){
                    stistat = 1;
                }else{
                    dispstr = "You cannot search while editing.";
                }
            }
            ImGui::SameLine();
            if(ImGui::Button("Sort by Money##statistics")){
                if(!CHKSTAT(uistat_level1)){
                    stistat = 2;
                }else{
                    dispstr = "You cannot search while editing.";
                }
            }
            ImGui::SameLine();
            if(ImGui::Button("Unpaid Loan##statistics")){
                if(!CHKSTAT(uistat_level1)){
                    stistat = 3;
                }else{
                    dispstr = "You cannot search while editing.";
                }
            }
            if(ImGui::Button("Clear##statistics")){
                stistat = 0;
            }
            if(CHKSTAT(uistat_level1)){
                stistat = 0;
            }
            ImGui::SameLine();
            if(ImGui::Button("Close##statistics")){
                CLRSTAT(uistat_statistic);
            }
            if(stistat == 1){
                qListDescriptor srtdesc;
                qList_initdesc(srtdesc);
                qList_foreach(*data,iter){
                    Level1Entry *le = (Level1Entry*)iter->data;
                    SoldNumSort tmpsrt;
                    memcpy(&(tmpsrt.data),&(le->data),sizeof(Level1));
                    tmpsrt.sorter = le->ld.size;
                    qList_push_back(srtdesc,tmpsrt);
                }
                bubble_sort(&srtdesc,int_decre_sort);
                ImGui::Columns(3,"SortByNum");
                ImGui::Separator();
                GUICOLUMNNEXT("CarId");
                GUICOLUMNNEXT("CarName");
                GUICOLUMNNEXT("SoldNum");
                ImGui::Separator();
                qList_foreach(srtdesc,iter){
                    SoldNumSort *sns = (SoldNumSort*)iter->data;
                    GUICOLUMNNEXT("%s",sns->data.carId);
                    GUICOLUMNNEXT("%s",sns->data.carName);
                    GUICOLUMNNEXT("%d",sns->sorter);
                }
                ImGui::Columns(1);
            }else if(stistat == 2){
                qListDescriptor srtdesc;
                qList_initdesc(srtdesc);
                qList_foreach(*data,iter){
                    Level1Entry *le = (Level1Entry*)iter->data;
                    SoldMonSort tmpsrt;
                    memcpy(&(tmpsrt.data),&(le->data),sizeof(Level1));
                    tmpsrt.sorter = 0.0f;
                    qList_foreach(le->ld,iiter){
                        Level2Entry *lle = (Level2Entry*)iiter->data;
                        tmpsrt.sorter += lle->data.priceSum;
                    }
                    qList_push_back(srtdesc,tmpsrt);
                }
                bubble_sort(&srtdesc,float_decre_sort);
                ImGui::Columns(3,"SortByMon");
                ImGui::Separator();
                GUICOLUMNNEXT("CarId");
                GUICOLUMNNEXT("CarName");
                GUICOLUMNNEXT("SoldMon");
                ImGui::Separator();
                qList_foreach(srtdesc,iter){
                    SoldMonSort *sns = (SoldMonSort*)iter->data;
                    GUICOLUMNNEXT("%s",sns->data.carId);
                    GUICOLUMNNEXT("%s",sns->data.carName);
                    GUICOLUMNNEXT("%.2f",sns->sorter);
                }
                ImGui::Columns(1); 
            }else if(stistat == 3){
                ImGui::Separator();
                ImGui::Columns(3,"LoanRemains");
                GUICOLUMNNEXT("CarId");
                GUICOLUMNNEXT("CustName");
                GUICOLUMNNEXT("Remain");
                ImGui::Separator();
                qList_foreach(*data,iter){
                    Level1Entry *tmple = (Level1Entry*)iter->data;
                    if(tmple->ld.size != 0){
                        qList_foreach(tmple->ld,iiter){
                            Level2Entry *tmplle = (Level2Entry*)iiter->data;
                            if(tmplle->ld.size != 0){
                                float tmpremain=99999999.0f;
                                qList_foreach(tmplle->ld,iiiter){
                                    Level3Entry *le = (Level3Entry*)iiiter->data;
                                    if(le->data.remain < tmpremain){
                                        tmpremain = le->data.remain;
                                    }
                                }
                                if(tmpremain > fepsilon){
                                    GUICOLUMNNEXT("%s",tmplle->data.carId);
                                    GUICOLUMNNEXT("%s",tmplle->data.customerName);
                                    GUICOLUMNNEXT("%.2f",tmpremain);
                                }
                            }
                        }
                    }
                }
                ImGui::Columns(1);
            }
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
    // stop networking processes
    Messeage shutm;
    shutm.qid = 99;
    shutm.payload = qbss_new();
    LOCKNET;
    qList_push_back(network_notifier,shutm);
    UNLOCKNET;
    while(1){
        // wait network successfully shutdown
        LOCKUI;
        if(ui_notifier.tail != NULL){
            Messeage *m = (Messeage*)ui_notifier.tail->data;
            if(m->qid == 255){
                UNLOCKUI;
                break;
            }
        }
        UNLOCKUI;
    }
    // Cleanup
    ImGui_ImplGlfw_Shutdown();
    glfwTerminate();

    return 0;
}
