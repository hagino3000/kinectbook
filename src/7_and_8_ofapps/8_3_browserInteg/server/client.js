var socket;

// 画面ロード時の処理
$(function() {
  // canvasの初期化
  var canvas = document.getElementById('canvas');
  ctx = canvas.getContext && canvas.getContext('2d');
  if (ctx) {
    ctx.globalCompositeOperation = "source-over";
    ctx.fillStyle = "rgba(0,0,0,1)";
    ctx.fillRect(0,0, width, height);
  } else {
    throw "Failed at initialize canvas context";
  }

  // WebSocketの設定
  socket = new io.Socket('localhost', {
    port : 8080
  });
  socket.on('connect', handleConnect);
  socket.on('message', handleMessage);
  socket.on('disconnect', handleDisconnect);
  socket.connect();

  // イベントの設定
  $('#upBtn').click(function() {
    // upボタンが押された時にはサーバーに"UP_ANGLE"を送信する
    socket.send("UP_ANGLE");
  });
  $('#downBtn').click(function() {
    // downボタンが押された時にはサーバーに"DOWN_ANGLE"を送信する
    socket.send("DOWN_ANGLE");
  });
});

/**
 * WebSocket接続成功時の処理
 */
function handleConnect() {
  console.info('open socket!!');
  // 深度情報のリクエストを送信する
  socket.send('GET_DEPTH');
}

/**
 * WebSocket切断時の処理
 */
function handleDisconnect() {
  console.info('close socket');
}

/**
 * メッセージ受信時の処理
 */
function handleMessage(data) {
  try {
    var d = JSON.parse(data);
    var eventName = d.type.toLowerCase();
    if (eventName === 'depth') {
      draw(d);
    } else if (eventName === 'tilt') {
      setTiltValue(d);
    } else {
      console.info('Unknown event:', eventName);
    }
  } catch(e) {
    console.dir(data);
    console.error(e);
  }
}

/**
 * チルト角変更時の処理
 */
function setTiltValue(data) {
  $('#angle').text('Current Angle: ' + data.angle);
}

// 深度は320px * 240px で描画する
var width = 320, height = 240, ctx;

/**
 * 深度画像の描画処理
 */
function draw(rowData) {
  if (ctx) {
    // カンマ区切りの深度情報を配列に戻す
    var dataArr = rowData.data.split(',');
    // 描画するサイズ分のImageDataを作成
    var imageData = ctx.createImageData(width, height);
    var pixels = imageData.data;
    var size = width * height;

    for (var i = 0; i < size; i++) {
      var depth = dataArr[i];
      // それっぽい色をつける。
      pixels[i*4]   /*R*/ = 0;
      pixels[i*4+1] /*G*/ = Math.max(0, depth*2-255);
      pixels[i*4+2] /*B*/ = Math.max(0, 255-depth*2);
      pixels[i*4+3] /*A*/ = 255;
    }
    // canvasに反映させる
    ctx.putImageData(imageData, 0, 0);
  }

  // 新規保存画像を表示する
  var imageHolder = $('#images'); 
  rowData.imageFiles.forEach(function(fNo) {
    var fileName = '../bin/data/' + fNo + '.png';
    var img = $('<img>').attr('src', fileName);
    img.prependTo(imageHolder);

    // 10個より増えた時は古い物から消す
    if ($('#images > img').length > 10) {
      $('#images > img').last().remove();
    }
  });

  // 50ms後に再取得
  setTimeout(function() {
    socket.send('GET_DEPTH');
  }, 50);
}

