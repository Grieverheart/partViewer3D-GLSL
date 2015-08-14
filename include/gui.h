#ifndef __GUI_H
#define __GUI_H

typedef struct CTwBar TwBar;
class Scene;

class Gui{
public:
    Gui(Scene* scene, int width, int height);
    ~Gui(void);
    void resize(int width, int height);
    void draw(void)const;
private:
    TwBar* bar_;
    Scene* scene_;
};

#endif
