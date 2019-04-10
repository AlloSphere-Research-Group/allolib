#ifndef SOUNDFILERECORDGUI_H
#define SOUNDFILERECORDGUI_H


#include "al/util/ui/al_ParameterGUI.hpp"

#include "al/soundfile/al_OutputRecorder.hpp"


namespace al
{

class SoundFileRecordGUI
{
public:

  static void drawRecorderWidget(OutputRecorder *recorder, double frameRate, uint32_t numChannels, uint32_t bufferSize = 0) {

    struct SoundfileRecorderState {
        bool recordButton;
        bool overwriteButton;
    };
    static std::map<SoundFileBufferedRecord *, SoundfileRecorderState> stateMap;
    if(stateMap.find(recorder) == stateMap.end()) {
        stateMap[recorder] = SoundfileRecorderState{0, false};
    }
    SoundfileRecorderState &state = stateMap[recorder];
    ImGui::PushID(std::to_string((unsigned long) recorder).c_str());
    if (ImGui::CollapsingHeader("Record Audio", ImGuiTreeNodeFlags_CollapsingHeader | ImGuiTreeNodeFlags_DefaultOpen)) {
        static char buf1[64] = "test.wav"; ImGui::InputText("Record Name", buf1, 63);
        if (state.recordButton) {
          ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0, 0.0, 0.0, 1.0));
          ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0, 0.5, 0.5, 1.0));
        }
        std::string buttonText = state.recordButton ? "Stop" : "Record";
        bool recordButtonClicked = ImGui::Button(buttonText.c_str());
        if (state.recordButton) {
          ImGui::PopStyleColor();
          ImGui::PopStyleColor();
        }
        if (recordButtonClicked) {
            state.recordButton = !state.recordButton;
            if (state.recordButton) {
              uint32_t ringBufferSize;
              if (bufferSize == 0) {
                ringBufferSize = 8192;
              } else {
                ringBufferSize = bufferSize * numChannels * 4;
              }
              std::string filename;
              if (!state.overwriteButton) {
                filename = buf1;
                int counter = 0;
                while(File::exists(filename) && counter < 9999) {
                  filename = buf1;
                  int lastDot = filename.find_last_of(".");
                  filename = filename.substr(0, lastDot) + std::to_string(counter++) + filename.substr(lastDot);
                }
              }

              if (!recorder->start(filename, frameRate, numChannels, ringBufferSize)) {
                std::cerr << "Error opening file for record" << std::endl;
              }
            } else {
                recorder->close();
            }
        }
        ImGui::SameLine();
        ImGui::Checkbox("Overwrite", &state.overwriteButton);
    }
    ImGui::PopID();
  }

};

} // namespace al

#endif // SOUNDFILERECORDGUI_H
