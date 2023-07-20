# faceRecognitionAndSoundPlayUsingArduino-JetsonNano
When Jetson nano performs facial recognition and finds a learner, it signals Arduino Uno via serial communication and Arduino Uno receives it to generate the music that was originally written.

# Abstract
Using Jetson nono and Arduino, we made a simple face recognition type sound reproduction machine.
It can be used for something interesting by playing music associated with a person's face.
Currently, the machine learns one person's face and plays music accordingly.
(The code is designed to be able to learn multiple people.)
* Demonstration Video
  * https://youtu.be/NSpYV-hk05w

# Requirement
* [Jetson Nano Developer Kit](https://developer.nvidia.com/embedded/jetson-nano-developer-kit)
  * [Finish the setup and get Ubuntu working](https://developer.nvidia.com/embedded/learn/get-started-jetson-nano-devkit)
* [MAKER-UNO](https://futuranet.it/futurashop/image/catalog/data/Download/Maker%20Uno%20User's%20Manual.pdf)
* web camera
* Python3.11.4
  ```
    pip install opencv-python
    pip install insightface
    pip install numpy
    pip install facenet-pytorch
    pip install pickle-mixin
    pip install scikit-learn
    pip install facenet-pytorch
    ```
    

※ recommend that you define a command to invoke 'Python 3.11' and use python3.11 -m pip install ~
```
cd /usr/bin
sudo ln -s /usr/local/bin/python3.x python3.x

python3.11 -m pip install numpy
```
[If you get an error in imshow...](https://qiita.com/tik26/items/a75e03e523926cd2f059)


# Implementation Procedure
- 
