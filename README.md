# Qt Music Player

**A lightweight, interactive music player built with the Qt Framework and iTunes API**

---

## Project Overview

This application is a fully functional music player that allows users to:

- Search for songs and albums  
- Play previews of selected songs  
- Manage a personalized playlist  
- Control playback with an intuitive media toolbar

The player is built using the **Qt Framework** and utilizes the **iTunes API** for fetching music data and audio previews. It prioritizes a user-friendly interface, responsive design, and scalable architecture.

---

## 🧩 Features

### Search and Playback

- Users can search by song or album using a text input field.
- Fetches data from the **iTunes Search API**.
- Automatically retrieves and plays audio previews using `QMediaPlayer`.

### Playlist Management

- Add and remove songs from a dynamic playlist.
- Built using `QToolBar` and `QListWidget`.
- Songs can be navigated and selected directly from the playlist.
- Playlist includes "Next" and "Previous" controls.

### Media Controls

- Core playback features: **Play**, **Pause**, **Next**, **Previous**, and **Volume**.
- Interactive slider shows and controls the song’s current play position.
- All actions update the GUI and print feedback using `QDebug`.

---
## iTunes API Integration

**Base URL:**  
`https://itunes.apple.com/search`

**Key Parameters:**
- `term`: User search input (e.g., artist or song name)
- `entity`: Set to `song` to fetch track previews

**Fields Used in JSON Response:**
- `"trackName"` – Song title  
- `"artistName"` – Artist name  
- `"collectionName"` – Album title  
- `"previewUrl"` – Audio preview URL  
- `"artworkUrl100"` – Album artwork  

---

## 🧪 Testing & Output

### Search Testing
- Artist name  
- Track name  
- Combined queries  

### Playlist Testing
- Add/remove stability  
- No crashes on edge cases  
- Valid UI and media player sync  

### Playback Validation
- Slider control works as intended for 30-second previews  
- All actions logged to the terminal via `QDebug` for transparency  

### Interface Highlights
- Playlist displayed at the center of the screen  
- Clickable track selection  
- Toolbar with full media playback controls  
- Album artwork shown next to song info  

---

## Tech Stack

- Qt 5 or Qt 6 (Widgets, Network, Multimedia)  
- C++  
- iTunes Search API  
- JSON parsing via Qt’s `QJsonDocument` and `QJsonObject`  

