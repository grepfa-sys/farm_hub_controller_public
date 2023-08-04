# esp

# esp가 가지는 하위 모듈
1. 모터 100 (방향제어, 방향젲어)
   2. 방향제어 -농업용모터
   3. 방향제어 x - AC 릴레이, 직류 릴레이
      4. ON/OFF (Bool) Direction (Bool)
5. 센서 20
   6. 센서1. uint16
   7. 센서2. uint16

- 공통
  - 타입 
  - 버전 
  

# 제공되는 정보
- 센서값1 uint16 - Sensor1
- 센서값2 uint16
- 
- 타입 uint16
- 버전 uint16
- 기기의 시간
- 등등 메타데이터


# 요구하는 정보
- on/off bool
- 방향 bool


mqtt shadow

어느토픽

## $aws/things/{thingName}/shadow/name/channels/update/accepted
## $aws/things/{thingName}/shadow/name/channels/update

 - thingName aws 기기 이름

```json
{
"state": {
    "reported": {
        "sensor1": 18,
        "sensor2": 19
    }
  }
}
```
sensor2  를없에고 싶으면 null을 넣어라

