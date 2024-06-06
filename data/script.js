const websocket = new WebSocket(`ws://${window.location.hostname}/ws`);

websocket.onmessage = function(event) {
  const data = JSON.parse(event.data);
  if (data.medida !== undefined) {
    const medidaGramas = data.medida * 1000; 
    document.getElementById('medida_atual').innerText = medidaGramas.toFixed(0); 

    const now = new Date();
    addDataToChart(now.toLocaleTimeString(), medidaGramas);
  }
};

function addSchedule() {
  const schedulesDiv = document.getElementById('schedules');
  const scheduleDiv = document.createElement('div');
  scheduleDiv.className = 'schedule-group';
  scheduleDiv.innerHTML = `
    <input type="text" placeholder="Nome do horário">
    <div class="form-group">
      <label>Horário:</label>
      <input type="time">
    </div>
    <button class="remove-btn" onclick="removeSchedule(this)">Remover horário</button>
  `;
  schedulesDiv.appendChild(scheduleDiv);
}

function removeSchedule(button) {
  button.parentElement.remove();
}

function saveSchedules() {
  const schedules = [];
  const scheduleGroups = document.getElementsByClassName('schedule-group');
  for (const group of scheduleGroups) {
    const name = group.querySelector('input[type="text"]').value;
    const time = group.querySelector('input[type="time"]').value.split(':');
    const hour = parseInt(time[0]);
    const minute = parseInt(time[1]);
    const second = 0;
    schedules.push({ name, hour, minute, second });
  }
  websocket.send(JSON.stringify({ action: 'saveSchedules', schedules }));
}

function moveServo() {
  const position = document.getElementById('servoPosition').value;
  if (position >= 0 && position <= 180) {
    websocket.send(`moveServo?servo=1&position=${position}`);
  } else {
    alert("Por favor, insira um valor entre 0 e 180.");
  }
}

function moveServoForDuration(seconds) {
  const position = document.getElementById('servoPosition').value;
  if (position >= 0 && position <= 180) {
    websocket.send(`moveServoForDuration?servo=1&position=${position}&duration=${seconds}`);
  } else {
    alert("Por favor, insira um valor entre 0 e 180.");
  }
}

const ctx = document.getElementById('medidaChart').getContext('2d');
const medidaChart = new Chart(ctx, {
  type: 'line',
  data: {
    labels: [],
    datasets: [{
      label: 'Ração no pote (g)',
      data: [],
      borderColor: 'rgba(75, 192, 192, 1)',
      borderWidth: 2,
      fill: false
    }]
  },
  options: {
    scales: {
      x: {
        title: {
          display: true,
          text: 'Tempo'
        }
      },
      y: {
        title: {
          display: true,
          text: 'Quantidade (g)'
        }
      }
    }
  }
});

function addDataToChart(time, medida) {
  medidaChart.data.labels.push(time);
  medidaChart.data.datasets[0].data.push(medida);
  medidaChart.update();
}
