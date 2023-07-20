import cv2
import insightface
import numpy as np
from facenet_pytorch import MTCNN, InceptionResnetV1
import pickle
import serial  # Add this line

# Define variables
video_url = 0 # use webcam
FACE_THRESHOLD = 0.5 # 閾値
# Initialize InsightFace detector
insightface_detector = insightface.app.FaceAnalysis()
insightface_detector.prepare(ctx_id=-1)  # Use CPU
# Initialize the MTCNN module
mtcnn = MTCNN()
# Initialize a pre-trained InceptionResnetV1 model
resnet = InceptionResnetV1(pretrained='vggface2').eval()
# Load the trained SVM model
with open('svm_model.pkl', 'rb') as f:
    model = pickle.load(f)
cap = cv2.VideoCapture(video_url)
# Initialize serial communication
ser = serial.Serial('/dev/ttyUSB0', 9600) # Change this to your serial port
# Initialize the flag
sent_serial = False

# face recognition
while True:
    ret, frame = cap.read()
    if not ret:
        break

    faces = insightface_detector.get(frame)

    for i, face in enumerate(faces):
        x1, y1, x2, y2 = face.bbox.astype(int)
        x1 = max(0, x1)
        y1 = max(0, y1)
        x2 = min(frame.shape[1] - 1, x2)
        y2 = min(frame.shape[0] - 1, y2)

        # Extract the face from the frame
        face_img = frame[y1:y2, x1:x2]
        face_cropped = mtcnn(face_img)
        if face_cropped is not None:
            face_embedding = resnet(face_cropped.unsqueeze(0))

            # Predict the label of the face
            proba = model.predict_proba(face_embedding.detach().numpy())
            max_proba = np.max(proba)
            if max_proba > FACE_THRESHOLD:
                label = model.predict(face_embedding.detach().numpy())
            else:
                label = ['Unknown']

            # If the label is ' ' and the flag is False, send a message through serial communication
            # if you search 'John' : if label[0] == 'John'
            if label[0] == 'person_1' and not sent_serial:
                #print("detect")
                ser.write(b'detected\n')
                sent_serial = True
            
            # for verification purposes
            # Draw rectangle around the face and put label
            cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), 2)
            cv2.putText(frame, label[0], (x1, y1-10), cv2.FONT_HERSHEY_SIMPLEX, 0.9, (0,255,0), 2)
    cv2.imshow('Video', frame)

    # Break the loop on 'q' key press
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
ser.close()  # Close the serial communication
