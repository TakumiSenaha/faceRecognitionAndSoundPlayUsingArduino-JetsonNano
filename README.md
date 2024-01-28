# faceRecognitionAndSoundPlayUsingArduino-JetsonNano
When Jetson nano performs facial recognition and finds a learner, it signals Arduino Uno via serial communication and Arduino Uno receives it to generate the music that was originally written.

# Abstract
<img src="/images/setting.png" width=240> <img src="/images/faceRecognition.png" width=240>


Using Jetson Nono and Arduino, we made a simple face recognition type sound reproduction machine.
It can be used for something interesting by playing music associated with a person's face.
Currently, the machine learns one person's face and plays music accordingly.
([The code](/Python/faceTrainFaceNet.py) is designed to be able to learn multiple people.)
* Demonstration Video
  * https://youtu.be/NSpYV-hk05w

# Requirement
* [Jetson Nano Developer Kit](https://developer.nvidia.com/embedded/jetson-nano-developer-kit)
  * [Finish the setup and get Ubuntu working](https://developer.nvidia.com/embedded/learn/get-started-jetson-nano-devkit)
* [MAKER-UNO](https://futuranet.it/futurashop/image/catalog/data/Download/Maker%20Uno%20User's%20Manual.pdf)
* web camera
* [Arduino IDE for Linux](https://docs.arduino.cc/software/ide-v1/tutorials/Linux)
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
    

※ recommend that you define a command to invoke `Python 3.11` and use `python3.11 -m pip install ~`
```
cd /usr/bin
sudo ln -s /usr/local/bin/python3.x python3.x

python3.11 -m pip install numpy
```
[If you get an error in imshow...](https://qiita.com/tik26/items/a75e03e523926cd2f059)


# Implementation Procedure
1. Stores face images for training.


   The [train_dir](/Python/train_dir/) directory structure is shown below, where several face images are stored. (Recognition of as few as 1 photos is possible.)
   ```
   <train_dir>/
            <person_1>/
                <person_1_face-1>.jpg
                <person_1_face-2>.jpg
                .
                .
                <person_1_face-n>.jpg
           <person_2>/
                <person_2_face-1>.jpg
                <person_2_face-2>.jpg
                .
                .
                <person_2_face-n>.jpg
            .
            .
            <person_n>/
                <person_n_face-1>.jpg
                <person_n_face-2>.jpg
                .
                .
                <person_n_face-n>.jpg
   ```
   (The name of each directory will be the name returned as a result of the judgment.)
2. Then run [faceTrainFaceNet.py](/Python/faceTrainFaceNet.py) to train faces. A file named `svm_model.pkl` is generated.
   ```
   python3.11 faceTrainFaceNet.py
   ```
3. Write [midi-player.ino](/ArduinoUno/midi-player/midi-player.ino) to Arduino Uno (MAKER-UNO). (Select your serial port appropriately. In some cases, you may get an error saying you do not have permission.)
   >>Show the connection where serial communication is possible ("/dev/ttyUSB0" in my case)
   >>```
   >>ls -l /dev/ttyUSB0
   >>```
   >>Output result
   >>
   >>`crw-rw---- 1 root uucp 188, 0 Aug 11 13:33 /dev/ttyUSB0`
   >>
   >>If the Arduino IDE says you don't have permissions to write to the device, you can use the following command
   >>```
   >>chmod 666 /dev/ttyUSB0
   >>```
4. Connect a USB webcam to the Jetson Nano and run [faceRecognition.py](/Python/faceRecgnition.py).
   ```
   sudo python3.11 Python/faceRecognition.py
   ```
   The person to be judged can be changed by rewriting the label judgment part of this code.In [faceRecognition.py](/Python/faceRecgnition.py), the label returned by the authentication result is the name of the directory where the person's face image is stored. In this case, it is something like `person_n`.
   ```
   if label[0] == 'person_1' and not sent_serial:
                #print("detect")
                ser.write(b'detected\n')
                sent_serial = True
   ```
5. When a face is detected, the Arduino makes a sound.
<p align="center">
    <img src="/images/recognitionAndSound.png" width=480>
</p>



