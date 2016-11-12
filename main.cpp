/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <gilles.fernandez10#gmail.com> wrote this file. As long as you retain this 
 * notice you can do whatever you want with this stuff. If we meet some day, 
 * and you think this stuff is worth it, you can buy me a beer in return.
 * Gilles Fernandez
 * ----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <vlc/vlc.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

class Player {
public:
    Player() : m_mediaUrl(""), m_index(0), m_playing(false) {
        
	m_mediaList.push_back("http://ordipapy-lapremiere.gfernandez.be");
	m_mediaList.push_back("http://ordipapy-franceinter.gfernandez.be");
	m_mediaList.push_back("http://ordipapy-franceculture.gfernandez.be");
	m_mediaList.push_back("http://ordipapy-europe1.gfernandez.be");
	m_mediaList.push_back("http://ordipapy-musiq3.gfernandez.be");
	m_mediaList.push_back("http://ordipapy-francemusique.gfernandez.be");

        m_instance = libvlc_new (0, NULL);
        
        reset();
    }
    
    void play() {
        if (m_playing == true)
            return;
        
        m_playing = true;
        refresh();
        libvlc_media_player_play(m_mediaPlayer);
    }
    
    void stop() {
        if (m_playing == false)
            return;
        
        m_playing = false;
        libvlc_media_player_stop(m_mediaPlayer);
        reset();
    }
    
    void next() {
	if (m_playing == false)
	    return;

        libvlc_media_player_stop(m_mediaPlayer);
        if (m_index < m_mediaList.size() - 1)
            m_index++;
        else
            m_index = 0;
        
        refresh();
        libvlc_media_player_play(m_mediaPlayer);
    }
    
    void previous() {
	if (m_playing == false)
	    return;

        libvlc_media_player_stop(m_mediaPlayer);
        if (m_index > 0)
            m_index--;
        else
            m_index = m_mediaList.size() - 1;
        
        refresh();
        libvlc_media_player_play(m_mediaPlayer);
    }
    
    void reset() {
        m_index = 0;
        refresh();
    }
    
    void refresh() {
        m_mediaUrl = m_mediaList[m_index];
        
        m_media = libvlc_media_new_location(m_instance, m_mediaUrl.c_str());
        m_mediaPlayer = libvlc_media_player_new_from_media(m_media);
        libvlc_media_release(m_media);
    }
    
    bool playing() const { return m_playing; }
    
protected:
    libvlc_instance_t * m_instance;
    libvlc_media_player_t *m_mediaPlayer;
    libvlc_media_t *m_media;
    std::string m_mediaUrl;
    std::vector<std::string> m_mediaList;
    int m_index;
    bool m_playing;
};

class GPIO {
public:
    GPIO(std::string num) : m_num(num) {}
    
    // export is a keyword in c :(
    bool exportGPIO() {
        std::string exportPath = "/sys/class/gpio/export";
        
        std::ofstream exportFile(exportPath.c_str());
        if (exportFile < 0) {
            std::cerr << "OPERATION FAILED: Unable to export GPIO " << m_num << "." <<std::endl;
            return false;
        }
        
        exportFile << m_num;
        exportFile.close();
        return true;
    }
    
    bool unexportGPIO() {
        std::string exportPath = "/sys/class/gpio/unexport";
        
        std::ofstream exportFile(exportPath.c_str());
        if (exportFile < 0) {
            std::cerr << "OPERATION FAILED: Unable to unexport GPIO " << m_num << "." << std::endl;
            return false;
        }
        
        exportFile << m_num;
        exportFile.close();
        return true;
    }
    
    bool setDirection(std::string direction) {
        std::stringstream ss;
        ss << "/sys/class/gpio/gpio" << m_num << "/direction";
        std::string directionPath = ss.str();
        
        std::ofstream directionFile(directionPath.c_str());
        if (directionFile < 0) {
            std::cerr << "OPERATION FAILED: Unable to set direction " << direction << " of GPIO " << m_num << "." << std::endl;
            return false;
        }
        
        directionFile << direction;
        directionFile.close();
    }
    
    bool setValue(std::string value) {
        std::stringstream ss;
        ss << "/sys/class/gpio/gpio" << m_num << std::string("/value");
        std::string valuePath = ss.str();
        
        std::ofstream valueFile(valuePath.c_str());
        if (valueFile < 0) {
            std::cerr << "OPERATION FAILED: Unable to set value " << value << " of GPIO " << m_num << "." << std::endl;
            return false;
        }
        
        valueFile << value;
        valueFile.close();
    }
    
    bool value(std::string& value) {
        std::stringstream ss;
        ss << "/sys/class/gpio/gpio" << m_num << "/value";
        std::string valuePath = ss.str();
        
        std::ifstream valueFile(valuePath.c_str());
        if (valueFile < 0) {
            std::cout << "OPERATION FAILED: Unable to get value " << value << "of GPIO " << m_num << "." << std::endl;
            return false;
        }
        
        valueFile >> value;
        
        valueFile.close();
        return true;
    }
    
    std::string num() const {
        return m_num;
    }
    
protected:
    std::string m_num;
};

class PushButton {
public:
    PushButton(std::string ledGpio, std::string buttonGpio)
    :
        m_led(ledGpio),
        m_button(buttonGpio),
        m_pressed(false),
        m_previousValue("0")
    {
        m_led.exportGPIO();
        m_button.exportGPIO();
        
        m_led.setDirection("out");
        m_button.setDirection("in");
    }
    
    void turnOn() {
        m_led.setValue("1");
    }
    
    void turnOff() {
        m_led.setValue("0");
    }
    
    void update() {
        std::string currentValue;
        m_button.value(currentValue);
        
        if (currentValue != m_previousValue) {
            if (currentValue != "0")
                pressed();
            else
                released();
        }
        
        m_previousValue = currentValue;
    }
    
    virtual void pressed() = 0;
    virtual void released() = 0;
    
protected:
    GPIO m_led;
    GPIO m_button;
    
    bool m_pressed;
    std::string m_previousValue;
};

class NextButton : public PushButton {
public:
    NextButton(Player* p) : PushButton("18", "17"), player(p) {}
    
    virtual void pressed() {
        player->next();
    }
    
    virtual void released() {}
    
protected:
    Player* player;
};

class PreviousButton : public PushButton {
public:
    PreviousButton(Player* p) : PushButton("23", "22"), player(p) {}
    
    virtual void pressed() {
        player->previous();
    }
    
    virtual void released() {}
    
protected:
    Player* player;
};

class ToggleButton {
public:
    ToggleButton(std::string gpio)
    :
        m_button(gpio),
        m_toggled(false),
        m_previousValue("1")
    {
        m_button.exportGPIO();
        m_button.setDirection("in");
    }
    
    bool toggled() const {
        return m_toggled;
    }
    
    void update() {
        std::string currentValue;
        m_button.value(currentValue);
        
        if (currentValue != m_previousValue) {
            m_toggled = currentValue == "0";
            toggledChanged();
        }
        
        m_previousValue = currentValue;
    }
    
    virtual void toggledChanged() = 0;
    
protected:
    GPIO m_button;
    bool m_toggled;
    std::string m_previousValue;
};

class OnOffButton : public ToggleButton {
public:
    OnOffButton(Player* p) : ToggleButton("4"), m_player(p) {
        update();
        toggledChanged();
    }
    
    virtual void toggledChanged() {
        if (toggled())
            m_player->play();
        else
            m_player->stop();
    }
    
protected:
    Player* m_player;
};

int main(int argc, char* argv[]) {
    sleep(5);

    Player p;
    
    NextButton next(&p);
    PreviousButton previous(&p);
    OnOffButton onOff(&p);
    
    while(1) {
        next.update();
        previous.update();
        onOff.update();
        
        if (p.playing()) {
            next.turnOn();
            previous.turnOn();
        }
        else {
            next.turnOff();
            previous.turnOff();
        }
        
        usleep(100000);
    }
 
    return 0;
}
