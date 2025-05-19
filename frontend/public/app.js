/** 
 * API URL pointing to the backend server exposed via ngrok.
 * Replace with the current ngrok URL or your local backend URL for testing.
 */
//const apiUrl = "https://c0b1-2806-261-484-b18-f388-fad9-1f72-76b2.ngrok-free.app/"; 

/** Uncomment this line for local testing with the backend */
const apiUrl = "http://localhost:3000/";

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

let selectedChipId = null;
let registeredDevices = {};

/** 
 * Fetches data from the backend server for a specified type (e.g., sensors or actuators).
 * Includes the "ngrok-skip-browser-warning" header to bypass ngrok's warning page.
 */
async function fetchData(type) {
  if (!selectedChipId) return null;
  const endpoint = `getLastData?chipId=${encodeURIComponent(selectedChipId)}&type=${type}`;
  const finalUrl = `${apiUrl}${endpoint}`;
  try {
    const response = await fetch(finalUrl, {
      headers: {
        "ngrok-skip-browser-warning": "true",
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
  card.classList.toggle("flipped");

  if (card.classList.contains("flipped")) {
    const historyList = card.querySelector(".history-list");
    historyList.innerHTML = "";

    const type = ["lmp", "pmp", "flt", "irr"].includes(key) ? "actuators" : "sensors";
    if (!selectedChipId) {
      historyList.innerHTML = "<div class='history-item'>No device selected.</div>";
      return;
    }
    const endpoint = `getHistoryData?chipId=${encodeURIComponent(selectedChipId)}&type=${type}&key=${key}`;
    const historyApiUrl = `${apiUrl}${endpoint}`;

    fetch(historyApiUrl)
      .then((response) => {
        if (!response.ok) throw new Error("Failed to fetch history data");
        return response.json();
      })
      .then((history) => {
        history.forEach((item) => {
          const value = item[key] !== undefined
            ? key === "ldr" ? (item[key] === "1" ? "Dark" : "Light")
            : key === "pir" ? (item[key] === "0" ? "None" : "Detected")
            : key === "pmp" ? (item[key] === "0" ? "OFF" : "ON")
            : key === "flt" ? (item[key] === "0" ? "NO" : "YES")
            : key === "lmp" ? (item[key] === "1" ? "ON" : "OFF")
            : key === "irr" ? (item[key] === "1" ? "ON" : "OFF")
            : item[key]
            : "N/A";

          const unit = key === "lvl" ? "%" : key === "tmp" ? "°C" : key === "hum" ? "%" : "";

          const date = new Date(item.timestamp);
          const formattedDate = date.toLocaleString();
          const div = document.createElement("div");
          div.className = "history-item";
          div.textContent = `${formattedDate}: ${value} ${unit}`;
          historyList.appendChild(div);
        });
      })
      .catch((error) => {
        console.error("Error fetching history data:", error);
        const errorDiv = document.createElement("div");
        errorDiv.className = "history-item";
        errorDiv.textContent = "Error loading history.";
        historyList.appendChild(errorDiv);
      });
  }
}

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
 * Fetch current settings from the backend and populate the settings menu
 */
async function loadSettings() {
  if (!selectedChipId) return;
  const endpoint = `getSettings?chipId=${encodeURIComponent(selectedChipId)}`;
  const settingsApiUrl = `${apiUrl}${endpoint}`;
  try {
    const response = await fetch(settingsApiUrl);
    if (!response.ok) throw new Error("Failed to fetch settings");
    const settings = await response.json();
    document.getElementById("min-level").value = settings.minLevel !== "NA" ? settings.minLevel : "";
    document.getElementById("max-level").value = settings.maxLevel !== "NA" ? settings.maxLevel : "";
    document.getElementById("low-humidity").value = settings.lowHumidity !== "NA" ? settings.lowHumidity : "";
    document.getElementById("hot-temperature").value = settings.hotTemperature !== "NA" ? settings.hotTemperature : "";
  } catch (error) {
    console.error("Error loading settings:", error);
    alert("Failed to load current settings. Please try again later.");
  }
}

/** Fetch registered devices from backend and populate selector */
async function loadRegisteredDevices() {
  try {
    const response = await fetch(`${apiUrl}getRegisteredDevices`);
    if (!response.ok) throw new Error("Failed to fetch registered devices");
    registeredDevices = await response.json();

    const deviceList = document.getElementById("device-list");
    deviceList.innerHTML = "";

    Object.entries(registeredDevices).forEach(([chipId, info]) => {
      const option = document.createElement("option");
      option.value = chipId;
      option.textContent = info.alias ? `${info.alias} (${chipId})` : chipId;
      deviceList.appendChild(option);
    });

    // Restore last selected device from localStorage, or select the first one
    const lastChipId = localStorage.getItem("selectedChipId");
    if (lastChipId && registeredDevices[lastChipId]) {
      deviceList.value = lastChipId;
      selectedChipId = lastChipId;
    } else if (deviceList.options.length > 0) {
      selectedChipId = deviceList.value;
    } else {
      selectedChipId = null;
    }
  } catch (error) {
    console.error("Error loading registered devices:", error);
  }
}

/** Register a new device */
async function registerDevice(chipId, alias) {
  try {
    const registeredAt = new Date().toISOString();
    const response = await fetch(`${apiUrl}registerDevice`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ chipId, alias, registeredAt }),
    });
    if (!response.ok) throw new Error("Failed to register device");
    await loadRegisteredDevices();
    document.getElementById("device-list").value = chipId;
    selectedChipId = chipId;
    localStorage.setItem("selectedChipId", chipId);
    alert("Device registered successfully!");
  } catch (error) {
    alert("Failed to register device. " + error.message);
  }
}

/** Call loadSettings when the page loads */
window.addEventListener("load", async () => {
  await loadRegisteredDevices();
  loadSettings();
  updateDashboard();
  setInterval(updateDashboard, 5000);
});

// Toggle settings menu visibility
document.querySelectorAll('.settings-icon').forEach(icon => {
  icon.addEventListener('click', (event) => {
    const group = event.target.closest('.group');
    const settingsMenu = group.querySelector('.settings-menu');
    settingsMenu.style.display = settingsMenu.style.display === 'none' ? 'block' : 'none';
  });
});

/** Toggle settings menu visibility */
const settingsIcon = document.getElementById("settings-icon");
const settingsMenu = document.getElementById("settings-menu");

settingsIcon.addEventListener("click", () => {
  if (settingsMenu) {
    const isHidden = settingsMenu.style.display === "none";
    settingsMenu.style.display = isHidden ? "block" : "none";

    if (isHidden) {
      loadSettings(); /** Reload settings when the menu is opened */
    }
  } else {
    console.error("Settings menu element not found.");
  }
});

// Handle form submissions for each group
document.querySelectorAll('.settings-menu form').forEach(form => {
  form.addEventListener('submit', (event) => {
    event.preventDefault();

    const groupId = form.id.split('-')[2]; // Extract group ID (e.g., "group1")
    const minValue = form.querySelector('input[name="min-value"]').value;
    const maxValue = form.querySelector('input[name="max-value"]').value;

    // Validate input
    if (parseFloat(minValue) >= parseFloat(maxValue)) {
      alert('Minimum value must be less than the maximum value.');
      return;
    }

    // Send settings to the server
    fetch('/savePumpSettings', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({ groupId, minValue, maxValue }),
    })
      .then(response => {
        if (!response.ok) throw new Error('Failed to save settings');
        alert('Settings saved successfully!');
      })
      .catch(error => {
        console.error('Error saving settings:', error);
        alert('Failed to save settings.');
      });
  });
});

/** 
 * Handle form submission for saving settings.
 */
const settingsForm = document.getElementById("settings-form");

settingsForm.addEventListener("submit", (event) => {
  event.preventDefault();
  if (!selectedChipId) {
    alert("Please select a device first.");
    return;
  }

  /** Get input values */
  const minLevelInput = document.getElementById("min-level");
  const maxLevelInput = document.getElementById("max-level");
  const lowHumidityInput = document.getElementById("low-humidity");
  const hotTemperatureInput = document.getElementById("hot-temperature");

  /** Prepare the userSettings object */
  const userSettings = {
    minLevel: minLevelInput.value ? parseFloat(minLevelInput.value) : "NA",
    maxLevel: maxLevelInput.value ? parseFloat(maxLevelInput.value) : "NA",
    lowHumidity: lowHumidityInput.value ? parseFloat(lowHumidityInput.value) : "NA",
    hotTemperature: hotTemperatureInput.value ? parseFloat(hotTemperatureInput.value) : "NA",
  };

  /** Validation logic */
  if ((userSettings.minLevel !== "NA" && userSettings.maxLevel === "NA") ||
      (userSettings.maxLevel !== "NA" && userSettings.minLevel === "NA")) {
    alert("If you change Min Level, you must also change Max Level, and vice versa.");
    return;
  }

  if ((userSettings.lowHumidity !== "NA" && userSettings.hotTemperature === "NA") ||
      (userSettings.hotTemperature !== "NA" && userSettings.lowHumidity === "NA")) {
    alert("If you change Low Humidity, you must also change Hot Temperature, and vice versa.");
    return;
  }

  /** Define the endpoint for saving settings */
  const endpoint = "saveSettings";
  const settingsApiUrl = `${apiUrl}${endpoint}`;

  /** Send settings to the server */
  fetch(settingsApiUrl, {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify({ chipId: selectedChipId, userSettings }),
  })
    .then((response) => {
      if (!response.ok) throw new Error("Failed to save settings");
      alert("Settings saved successfully!");
    })
    .catch((error) => {
      console.error("Error saving settings:", error);
      alert("Failed to save settings.");
    });
});

// Handle device selection change
document.getElementById("device-list").addEventListener("change", (e) => {
  selectedChipId = e.target.value;
  localStorage.setItem("selectedChipId", selectedChipId);
  loadSettings();
  updateDashboard();
});

// Handle Add Device button
document.getElementById("add-device-btn").addEventListener("click", () => {
  document.getElementById("add-device-modal").style.display = "flex";
});

// Handle modal close
document.getElementById("close-modal").onclick = () => {
  document.getElementById("add-device-modal").style.display = "none";
};

// Handle device registration form
document.getElementById("add-device-form").addEventListener("submit", async (e) => {
  e.preventDefault();
  const chipId = document.getElementById("chip-id-input").value.trim().toUpperCase();
  const alias = document.getElementById("alias-input").value.trim();
  if (!/^([0-9A-F]{2}:){5}[0-9A-F]{2}$/.test(chipId)) {
    alert("Invalid Chip ID format. Use XX:XX:XX:XX:XX:XX");
    return;
  }
  await registerDevice(chipId, alias);
  document.getElementById("add-device-modal").style.display = "none";
  loadSettings();
  updateDashboard();
});
