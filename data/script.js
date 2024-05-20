function updateServo1Time() {
    const hour = document.getElementById('servo1_hour').value;
    const minute = document.getElementById('servo1_minute').value;
    fetch(`/updateServo1Time?hour=${hour}&minute=${minute}`);
  }
  
  function updateServo2Time() {
    const hour = document.getElementById('servo2_hour').value;
    const minute = document.getElementById('servo2_minute').value;
    fetch(`/updateServo2Time?hour=${hour}&minute=${minute}`);
  }
  