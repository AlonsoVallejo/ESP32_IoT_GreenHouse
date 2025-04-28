/** 
 * API URL pointing to the backend server exposed via ngrok.
 * Replace with the current ngrok URL or your local backend URL for testing.
 */
//const apiUrl = "https://c0b1-2806-261-484-b18-f388-fad9-1f72-76b2.ngrok-free.app/getLastData"; 

/** Uncomment this line for local testing with the backend */
const apiUrl = "http://localhost:3000/getLastData";

/** 
 * History object to store up to the last 60 entries of sensor and actuator data
 * Key names correspond to various data points tracked by the system.
 */
const historyData = {
  ldr: [], /** Ambient Light */
  pir: [], /** Presence Detection */
  lvl: [], /** Water Level */
  tmp: [], /** Temperature */
  hum: [], /** Humidity */
  lmp: [], /** Lamp Status */
  flt: [], /** Sensor Fault Status */
  pmp: [], /** Pump Status */
  irr: [] /** Irrigation Status */
};

/** 
 * Fetches data from the backend server for a specified type (e.g., sensors or actuators).
 * Includes the "ngrok-skip-browser-warning" header to bypass ngrok's warning page.
 */
async function fetchData(type) {
  const finalUrl = `${apiUrl}?type=${type}`; /** Append the type query parameter to the API URL */
  try {
    const response = await fetch(finalUrl, {
      headers: {
        "ngrok-skip-browser-warning": "true", /** Bypass ngrok warning */
      },
    });
    if (!response.ok) throw new Error(`Failed to fetch ${type} data`);
    const data = await response.json();
    console.log(`Fetched ${type} data:`, data);
    return data;
  } catch (error) {
    console.error(`Error fetching ${type} data:`, error);
    return null;
  }
}

/** 
 * Returns the corresponding icon based on the data title and value provided.
 * Icons visually represent various data points, such as temperature or actuator status.
 */
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
    default: return '<i class="fas fa-microchip"></i>'; /** Default icon for unknown data types */
  }
}

/** 
 * Creates or updates a card element to display the latest data for a specific key.
 * Handles both the creation of new cards and the update of existing cards.
 */
function createOrUpdateCard(groupEl, title, unit, key, data) {
  const id = `card-${key}`;
  let card = document.getElementById(id);

  /** Determine the value to display in the card based on the data key */
  const value = data[key] !== undefined
    ? key === "ldr" ? (data[key] === "1" ? "Dark" : "Light")
    : key === "pir" ? (data[key] === "0" ? "None" : "Detected")
    : key === "pmp" ? (data[key] === "0" ? "OFF" : "ON")
    : key === "flt" ? (data[key] === "0" ? "NO" : "YES")
    : key === "lmp" ? (data[key] === "1" ? "ON" : "OFF")
    : key === "irr" ? (data[key] === "1" ? "ON" : "OFF")
    : data[key]
    : "N/A";

  /** Save the current data entry to history */
  if (data.timestamp) {
    historyData[key].push({ 
      timestamp: data.timestamp, 
      value: value, 
      unit: unit 
    });

    /** Keep only the last 60 entries in history */
    if (historyData[key].length > 60) {
      historyData[key].shift(); 
    }
  }

  if (!card) {
    /** Create a new card if one doesn't exist */
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
    /** Update the value in the existing card */
    const valueEl = card.querySelector(".value");
    const oldValue = valueEl.textContent;
    if (oldValue !== value) {
      valueEl.textContent = value;
      card.querySelector("i").outerHTML = getIcon(title, value);
      card.classList.add("flash"); /** Flash the card to indicate an update */
      setTimeout(() => card.classList.remove("flash"), 800);
    }
  }
}

/** 
 * Flips the card to display its history and ensures duplicate timestamps are removed.
 */
function flipCard(card, key) {
  card.classList.toggle('flipped');

  if (card.classList.contains('flipped')) {
    const historyList = card.querySelector('.history-list');
    historyList.innerHTML = "";

    /** Filter out duplicate timestamps and preserve unique history entries */
    const seenTimestamps = new Set();
    const uniqueHistory = [...historyData[key]].reverse().filter(item => {
      if (seenTimestamps.has(item.timestamp)) {
        return false; /** Ignore duplicate timestamps */
      } else {
        seenTimestamps.add(item.timestamp);
        return true; /** Include unique timestamp */
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

/** 
 * Fetches and updates the dashboard with the latest sensor and actuator data.
 */
/** 
 * Fetches and updates the dashboard with the latest sensor and actuator data.
 */
async function updateDashboard() {
  const g1 = document.getElementById("group1"); // Group for cards like Ambient Light, Presence, and Lamp
  const g2 = document.getElementById("group2"); // Group for cards like Water Level, Sensor Fault, and Pump
  const g3 = document.getElementById("group3"); // Group for cards like Temperature and Humidity
  const status = document.getElementById("status"); // Status display area

  let updateTime = null;

  /** Fetch and update sensor data */
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

  /** Fetch and update actuator data */
  const actuatorData = await fetchData("actuators");
  if (actuatorData) {
    console.log("Updating actuator data:", actuatorData);
    createOrUpdateCard(g1, "Lamp", "", "lmp", actuatorData);
    createOrUpdateCard(g2, "Sensor Fault", "", "flt", actuatorData);
    createOrUpdateCard(g2, "Pump", "", "pmp", actuatorData);
    createOrUpdateCard(g3, "Irrigation", "", "irr", actuatorData);
  }

  /** Update the status text to reflect the latest data */
  const statusIcon = document.getElementById("status-icon");
  const statusText = document.getElementById("status-text");

  if (updateTime) {
    const date = new Date(updateTime);
    const now = new Date();
    const elapsedMinutes = (now - date) / (1000 * 60); /** Calculate elapsed time in minutes */

    if (elapsedMinutes > 3) {
      /** If more than 3 minutes have passed since the last update */
      statusText.textContent = "Device connection error: Displaying last received data.";
      status.style.color = "#ff8888"; // Red for error
      statusIcon.className = "fas fa-exclamation-triangle";
      statusIcon.style.color = "#ff5555"; /** Red for error */
    } else {
      /** If the last update was within 3 minutes */
      statusText.textContent = `Last Update: ${date.toLocaleString()}`;
      status.style.color = "#aaa"; // Neutral gray
      statusIcon.className = "fas fa-check-circle";
      statusIcon.style.color = "#90ee90"; /** Green for success */
    }
  } else {
    /** If no update time is available */
    statusText.textContent = "Server/Internet connection error: Check server is running or WiFi connection.";
    status.style.color = "#ff8888"; // Red for error
    statusIcon.className = "fas fa-exclamation-triangle";
    statusIcon.style.color = "#ff5555"; /** Red for error */
  }
}

/** 
 * Set up the dashboard to update automatically every 5 seconds
 * after the page has loaded.
 */
window.addEventListener("load", () => {
  updateDashboard(); // Initial update on page load
  setInterval(updateDashboard, 5000); // Refresh the dashboard every 5 seconds
});
