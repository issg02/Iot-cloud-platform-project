// 중지를 위해 ID 보관
var intervalId = null;

// API 시작
function Start() {
    invokeAPI();
    intervalId = setInterval(invokeAPI, 1000);
    document.getElementById("result").innerHTML="조회 중...";
}
// API 중지

function Stop() {
    if(intervalId != null) {
        clearInterval(intervalId);
        document.getElementById("result").innerHTML="No Data";
    }
}

var invokeAPI = function() {
    // 디바이스 조회 URI
    // prod 스테이지 편집기의 맨 위에 있는 "호출 URL/devices"로 대체해야 함
    var API_URI = 'https://rmd9plw1m3.execute-api.ap-northeast-2.amazonaws.com/prod/devices/MyMKRWiFi1010';                 
    $.ajax(API_URI, {
        method: 'GET',
        contentType: "application/json",

        success: function (data, status, xhr) {
                var result = JSON.parse(data);
                printData(result);  // 성공시, 데이터 출력을 위한 함수 호출
                console.log("data="+data);
        },
        error: function(xhr,status,e){
                alert("error");
        }
    });
};

// 데이터 출력을 위한 함수
var printData = function(result){      
    document.getElementById("result").innerHTML = "Current distance : "+ result.state.reported.distance+
					     "\nCurrent state : " + result.state.reported.Currentstate;
}