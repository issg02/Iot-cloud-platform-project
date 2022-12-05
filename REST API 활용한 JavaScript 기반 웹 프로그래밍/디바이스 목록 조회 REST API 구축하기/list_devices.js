// API 시작
function Start() {
    invokeAPI();
    emptyTable();
}

var invokeAPI = function() {

    // 디바이스 조회 URI
    // prod 스테이지 편집기의 맨 위에 있는 "호출 URL/devices"로 대체해야 함
        var API_URI = 'https://rmd9plw1m3.execute-api.ap-northeast-2.amazonaws.com/prod/devices';                              
    $.ajax(API_URI, {
        method: 'GET',
        contentType: "application/json",

        success: function (data, status, xhr) {

            var result = JSON.parse(data);
            setDataList(result.things);  // 성공시, 데이터 출력을 위한 함수 호출
            console.log(data);
        },
        error: function(xhr,status,e){
          //  document.getElementById("result").innerHTML="Error";
            alert("error");
        }
    });
};

// 테이블 데이터 삭제
var emptyTable = function() {
    $( '#mytable > tbody').empty();
    document.getElementById("result").innerHTML="조회 중입니다.";
}

// 데이터 출력을 위한 함수
var setDataList = function(data){

    $( '#mytable > tbody').empty();
    data.forEach(function(v){

        var tr1 = document.createElement("tr");
        var td1 = document.createElement("td");
        var td2 = document.createElement("td");
        td1.innerText = v.thingName;
        td2.innerText = v.thingArn;
        tr1.appendChild(td1);
        tr1.appendChild(td2);
        $("table").append(tr1);
    })

    if(data.length>0){
            // 디바이스 목록 조회결과가 있는 경우 데이터가 없습니다 메시지 삭제
        document.getElementById("result").innerHTML="";
    } else if (data.length ==0) {
        document.getElementById("result").innerHTML="No Data";
    }
}