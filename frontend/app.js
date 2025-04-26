const apiUrl = "http://192.168.100.9:3000/getLastData";

const historyData = {
  ldr: [],
  pir: [],
  lvl: [],
  tmp: [],
  hum: [],
  lmp: [],
  flt: [],
  pmp: [],
  irr: []
};

async function fetchData(type) {
  try {
    const response = await fetch(`${apiUrl}?type=${type}`);
    if (!response.ok) throw new Error(`Failed to fetch ${type} data`);
    const data = await response.json();
    console.log(`Fetched ${type} data:`, data);
    return data;
  } catch (error) {
    console.error(`Error fetching ${type} data:`, error);
    return null;
  }
}

function getIcon(title, value) {
  switch (title) {
    case "Temperature": return '<i class="fas fa-thermometer-half"></i>';
    case "Humidity": return '<i class="fas fa-tint"></i>';
    case "Water Level": return '<i class="fas fa-water"></i>';
    case "Ambient Light": return value === "Dark" ? '<i class="fas fa-moon"></i>' : '<i class="fas fa-sun"></i>';
    case "Precense": return value === "Detected" ? '<i class="fas fa-walking"></i>' : '<i class="fas fa-user-slash"></i>';
    case "Lamp": return value === "ON" ? '<i class="fas fa-lightbulb"></i>' : '<i class="far fa-lightbulb"></i>';
    case "Pump": return value === "ON"
      ? '<i class="fas fa-faucet" style="color:lightblue;"></i>'
      : '<i class="fas fa-faucet" style="color:gray;"></i>';
    case "Sensor Fault": return value === "YES" ? '<i class="fas fa-exclamation-triangle"></i>' : '<i class="fas fa-check-circle"></i>';
    case "Irrigation": 
      return value === "ON" 
        ? '<i class="fas fa-cloud-showers-water" style="color:#5bc0de;"></i>' 
        : '<i class="fas fa-cloud-showers-water" style="color:gray;"></i>';
    default: return '<i class="fas fa-microchip"></i>';
  }
}

function createOrUpdateCard(groupEl, title, unit, key, data) {
  const id = `card-${key}`;
  let card = document.getElementById(id);

  const value = data[key] !== undefined
    ? key === "ldr" ? (data[key] === "1" ? "Dark" : "Light")
    : key === "pir" ? (data[key] === "0" ? "None" : "Detected")
    : key === "pmp" ? (data[key] === "0" ? "OFF" : "ON")
    : key === "flt" ? (data[key] === "0" ? "NO" : "YES")
    : key === "lmp" ? (data[key] === "1" ? "ON" : "OFF")
    : key === "irr" ? (data[key] === "1" ? "ON" : "OFF")
    : data[key]
    : "N/A";

  /** Save to history */
  if (data.timestamp) {
    historyData[key].push({ 
      timestamp: data.timestamp, 
      value: value, 
      unit: unit 
    });

    /** Keep only the last 60 entries */
    if (historyData[key].length > 60) {
      historyData[key].shift(); 
    }
  }

  if (!card) {
    /** Create a new card */
    card = document.createElement("div");
    card.className = "card";
    card.id = id;
    card.onclick = () => flipCard(card, key);
    card.innerHTML = `
      <div class="card-inner">
        <div class="card-front">
          ${getIcon(title, value)}
          <h3>${title}</h3>
          <p><span class="value">${value}</span> <span>${unit}</span></p>
        </div>
        <div class="card-back">
          <h4>History</h4>
          <div class="history-list" style="margin-top:10px;"></div>
        </div>
      </div>
    `;
    groupEl.appendChild(card);
  } else {
    /** Update value in existing card */
    const valueEl = card.querySelector(".value");
    const oldValue = valueEl.textContent;
    if (oldValue !== value) {
      valueEl.textContent = value;
      card.querySelector("i").outerHTML = getIcon(title, value);
      card.classList.add("flash");
      setTimeout(() => card.classList.remove("flash"), 800);
    }
  }
}

function flipCard(card, key) {
  card.classList.toggle('flipped');

  if (card.classList.contains('flipped')) {
    const historyList = card.querySelector('.history-list');
    historyList.innerHTML = "";

    /** Filter to remove duplicate timestamps */
    const seenTimestamps = new Set();
    const uniqueHistory = [...historyData[key]].reverse().filter(item => {
      if (seenTimestamps.has(item.timestamp)) {
        return false; /** Already seen */
      } else {
        seenTimestamps.add(item.timestamp);
        return true; /** First occurrence */
      }
    });

    uniqueHistory.forEach(item => {
      const date = new Date(item.timestamp);
      const formattedDate = date.toLocaleString();
      const div = document.createElement('div');
      div.className = "history-item";
      div.textContent = `${formattedDate}: ${item.value} ${item.unit}`;
      historyList.appendChild(div);
    });
  }
}

async function updateDashboard() {
  const g1 = document.getElementById("group1");
  const g2 = document.getElementById("group2");
  const g3 = document.getElementById("group3");
  const status = document.getElementById("status");

  let updateTime = null;

  /** Fetch sensor data */
  const sensorData = await fetchData("sensors");
  if (sensorData) {
    console.log("Updating sensor data:", sensorData);
    createOrUpdateCard(g1, "Ambient Light", "", "ldr", sensorData);
    createOrUpdateCard(g1, "Precense", "", "pir", sensorData);
    createOrUpdateCard(g2, "Water Level", "%", "lvl", sensorData);
    createOrUpdateCard(g3, "Temperature", "°C", "tmp", sensorData);
    createOrUpdateCard(g3, "Humidity", "%", "hum", sensorData);

    /** Save the timestamp if it exists */
    updateTime = sensorData.timestamp;
  }

  /** Fetch actuator data */
  const actuatorData = await fetchData("actuators");
  if (actuatorData) {
    console.log("Updating actuator data:", actuatorData);
    createOrUpdateCard(g1, "Lamp", "", "lmp", actuatorData);
    createOrUpdateCard(g2, "Sensor Fault", "", "flt", actuatorData);
    createOrUpdateCard(g2, "Pump", "", "pmp", actuatorData);
    createOrUpdateCard(g3, "Irrigation", "", "irr", actuatorData); 
  }

  /** Update status text */
  const statusIcon = document.getElementById("status-icon");
  const statusText = document.getElementById("status-text");

  if (updateTime) {
    const date = new Date(updateTime);
    const now = new Date();
    const elapsedMinutes = (now - date) / (1000 * 60); /** Calculate elapsed time in minutes */

    if (elapsedMinutes > 3) {
      /** If more than 3 minutes have passed since the last update */
      statusText.textContent = "Device connection error: Displaying last received data.";
      status.style.color = "#ff8888";
      statusIcon.className = "fas fa-exclamation-triangle";
      statusIcon.style.color = "#ff5555"; /** Red */
    } else {
      /** If the last update was within 3 minutes */
      statusText.textContent = `Last Update: ${date.toLocaleString()}`;
      status.style.color = "#aaa";
      statusIcon.className = "fas fa-check-circle";
      statusIcon.style.color = "#90ee90"; /** Green */
    }
  } else {
    /** If no update time is available */
    statusText.textContent = "Server/Internet connection error: Check server is running or WiFi connection.";
    status.style.color = "#ff8888";
    statusIcon.className = "fas fa-exclamation-triangle";
    statusIcon.style.color = "#ff5555"; /** Red */
  }
}

window.addEventListener("load", () => {
  updateDashboard();
  setInterval(updateDashboard, 5000);
});