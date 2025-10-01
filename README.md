# DirectX11-Flight

<br>

클라이언트까지만 만들고 DirectX12로 포팅하는 작업을 먼저하였기에 서버는 미완성으로 두었습니다. 

<br>

[유튜브 영상](https://youtu.be/oKvYUiVG-g8?si=Tr09Sb1_L0cLxAzb)


<br>

## 개발 개요

- **개발 인원** : 1인 개인 프로젝트
- **개발 언어** : C++, HLSL
- **개발 환경** : Visual Studio, DirectX11, PhysX

<br>

## 구현 목록

- **카메라**
  - 3인칭 카메라
  - 마우스 방향에 따른 시야 회전

<br>

- **비행**
  - 쿼터니언 기반의 회전 처리
  - 비행기는 카메라 방향을 따라가도록 보간(Slerp) 처리

<br>

- **빌보딩**
  - 알파 블렌딩을 이용한 빌보드 렌더링
  - 스프라이트 시트 uv 애니메이션

<br>

- **PhysX 라이브러리 연동**
  - PhysX 라이브러리를 활용한 충돌 처리

<br>

- **기타**
  - 큐브맵 SkyBox
  - Heightmap 지형
  - 직교 투영을 통한 UI 렌더링
  - Object Pooling 을 통한 총알 Mesh 생성 및 재사용
    
<br>
