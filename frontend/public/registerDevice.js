let selectedChipId = null;
let registeredDevices = {};

/**
 * Fetch registered devices from backend and populate selector
 */
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
      window.selectedChipId = lastChipId;
    } else if (deviceList.options.length > 0) {
      selectedChipId = deviceList.value;
      window.selectedChipId = selectedChipId;
    } else {
      selectedChipId = null;
      window.selectedChipId = null;
      localStorage.removeItem("selectedChipId"); // Clear localStorage if no devices are registered
      if (typeof loadSettings === "function") loadSettings();
      if (typeof updateDashboard === "function") updateDashboard();
    }
  } catch (error) {
    console.error("Error loading registered devices:", error);
  }
}

/**
 * Register a new device
 */
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
    window.selectedChipId = chipId;
    localStorage.setItem("selectedChipId", chipId);
    alert("Device registered successfully!");
  } catch (error) {
    alert("Failed to register device. " + error.message);
  }
}

// Handle device selection change
document.getElementById("device-list").addEventListener("change", (e) => {
  selectedChipId = e.target.value;
  window.selectedChipId = selectedChipId; // <-- Add this line!
  localStorage.setItem("selectedChipId", selectedChipId);
  if (typeof loadSettings === "function") loadSettings();
  if (typeof updateDashboard === "function") updateDashboard();
});

// Handle Add Device button
document.getElementById("add-device-btn").addEventListener("click", () => {
  document.getElementById("add-device-modal").style.display = "flex";
});

// Handle Remove Device button
document.getElementById("remove-device-btn").addEventListener("click", async () => {
  if (!selectedChipId) {
    alert("No device selected to remove.");
    return;
  }
  if (!confirm(`Are you sure you want to remove device ${selectedChipId}?`)) return;

  try {
    const response = await fetch(`${apiUrl}removeDevice`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ chipId: selectedChipId }),
    });
    if (!response.ok) throw new Error("Failed to remove device");
    alert("Device removed successfully!");
    await loadRegisteredDevices();
    if (typeof loadSettings === "function") loadSettings();
    if (typeof updateDashboard === "function") updateDashboard();
  } catch (error) {
    alert("Failed to remove device. " + error.message);
  }
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
  if (typeof loadSettings === "function") loadSettings();
  if (typeof updateDashboard === "function") updateDashboard();
});