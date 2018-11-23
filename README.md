# toyproject-firebase2esp8266

> firebase와 esp8266을 사용한 토이프로젝트  

## 디렉토리 구성  

**/webapp**: firebase 웹앱, iOS에 최적화  
**/esp8266**: firebase와 통신하는 디바이스  

## 기능  

1. firebase 웹앱의 Command 버튼을 통해 esp8266 조작  
2. esp8266의 GPIO핀에 연결된 센서 모듈의 데이터를 firebase RealtimeDB에 저장  
3. firebase RealtimeDB에 저장된 센서 값을 웹앱에서 확인 가능  