# ✨ Custom Rates

> *An independent community-driven level rating system for Geometry Dash*

---

## 🌟 What is Custom Rates?

**Custom Rates** is a Geode mod that brings a fully independent rating system
to Geometry Dash — separate from RobTop's official ratings.

The idea is simple: **anyone can submit a level**, and a team of trusted
moderators reviews and rates it. Rated levels appear in shared lists visible
to all players, complete with difficulty icons, rate types, and moderator info.

No waiting for official rates. No politics. Just good levels — found,
reviewed, and showcased by the community.

---

## 🚀 Features

### 📤 Level Submission
See a level that deserves a rate?
Open its page, hit the **custom rate button** on the left side,
and it gets added to the review queue instantly.
No forms, no external links — everything happens inside the game.

### 📋 Level Lists
Four different lists to explore:

| Button | Name | Description |
|--------|------|-------------|
| 📬 | **Sent** | Levels submitted and waiting for moderation |
| 🕐 | **Recent** | Latest rated levels, newest first |
| 🎲 | **Random** | A random selection of rated levels to discover |
| 👍 | **Top Likes** | Most liked rated levels pulled live from GD servers |

### 🎨 Level Page Integration
When you open a rated level, Custom Rates enriches its page with:
- A **custom difficulty icon** matching the moderator's rating
- A **"Rated by"** label showing which moderator approved the level

### 🛡️ Moderation Tools
Moderators get access to a dedicated rating panel:
- Choose **difficulty** (Auto → Extreme Demon)
- Choose **rate type** (Star / Featured / Epic / Legendary / Mythic)
- Set the **star count**
- **Delete** ratings when needed

---

## 🎮 How to Use

### Opening Custom Rates

1. Launch **Geometry Dash** and go to the main menu
2. Tap **Create** to open the Creator Menu
3. Find the **Featured** button — it has been replaced by Custom Rates
4. Tap it to open the **Custom Rates main screen**

---

### Navigating the Main Screen

The main screen shows four large buttons:
[ Sent ] [ Recent ]

[ Random ] [ Top Likes ]
Tap any of them to open the corresponding level list.
If you are a moderator, a **Delete** button will also appear
at the bottom — use it to remove a rating by entering the level ID.

---

### Browsing a Level List

Once inside a list (Sent, Recent, Random or Top Likes):

- **Scroll** the list up and down by **swiping** anywhere on it
- Navigate between pages using the **◀ arrow** on the left edge
  and the **▶ arrow** on the right edge of the screen
- Tap **🔄** in the **bottom-right corner** to refresh the list
  and reload fresh data from the server
- Each entry shows the **level name**, **moderator**, **difficulty icon**,
  and **rate type** (or likes count in Top Likes)
- Tap **View** on any entry to open that level's info page

---

### Submitting a Level

1. Find a level you think deserves a custom rate
2. Open its **Level Info** page (tap the level name)
3. Look for the **submit button** on the left side of the screen
   (flag icon)
4. Tap it — the level is instantly added to the **Sent** queue
5. A moderator will review it and either rate or skip it

> ℹ️ You can submit any level — online levels with a valid ID.
> Duplicate submissions are ignored automatically.

---

### Rating a Level *(Moderators only)*

1. Open any level's **Level Info** page
2. Tap the button on the left side — instead of submitting,
   it opens the **Rate Panel**
3. In the panel:
   - Use **＋ / −** to set the star count (1–20)
   - Tap the **difficulty icon** to cycle through difficulties
   - Tap the **rate type icon** to cycle through rate types
4. Tap **Rate!** to confirm
5. The level is removed from the Sent queue and added to rated lists

---

### Removing a Rating *(Moderators only)*

1. Open the **Custom Rates main screen**
2. Tap the red **Delete** button at the bottom
3. Enter the **Level ID** of the level you want to unrate
4. Tap **Delete** to confirm — the rating is removed from both
   the rated list and the sent queue

---

## 🏅 Rate Types

| Icon | Type | Description |
|------|------|-------------|
| ⭐ | **Star** | Standard community rate |
| 🔥 | **Featured** | Highlighted — great quality level |
| 💜 | **Epic** | Outstanding level, top tier |
| 👑 | **Legendary** | Exceptional — reserved for the very best |
| ✨ | **Mythic** | The highest honour — near-perfect levels |

---

## ⚙️ Difficulty Scale

Custom Rates uses the full GD difficulty spectrum:
Auto → Easy → Normal → Hard → Harder → Insane
→ Easy Demon → Medium Demon → Hard Demon
→ Insane Demon → Extreme Demon
Moderators pick the difficulty that best reflects the level's
actual challenge — independent of any official rating.

---

## ❓ FAQ

**Q: Does this affect official GD ratings?**
> No. Custom Rates runs on its own database and has zero interaction
> with RobTop's servers beyond fetching level names and like counts.

**Q: Can I submit my own level?**
> Yes! Anyone can submit any level, including their own.

**Q: How are moderators chosen?**
> Moderators are managed by the Custom Rates team directly
> in the database. There is no in-game application process.

**Q: The level shows "ID 12345" instead of a name — why?**
> This happens when the GD server doesn't return info for that level
> (private level, deleted, or server hiccup). Try refreshing.

**Q: I submitted a level — how do I know if it got rated?**
> Open the **Recent** list and look for your level there.
> You can also open the level's page and check for the
> "Rated by" label at the top.

**Q: Top Likes is slow to load — is that normal?**
> Yes. Top Likes fetches all rated levels from our database,
> then queries GD servers for live like counts, then sorts them.
> It takes a few seconds. Hit 🔄 to refresh manually.

---

## 📝 Changelog

### v1.0.0 — Initial Release
- 🎉 First public release of Custom Rates
- 📤 Level submission system via in-game button
- 📬 **Sent** list — review queue visible to all
- 🕐 **Recent** list — latest rated levels
- 🎲 **Random** list — discover random rated levels
- 👍 **Top Likes** — live ranking by GD like counts
- 🎨 Custom difficulty icons on rated level pages
- 🏷️ "Rated by" label on rated level pages
- 🛡️ Full moderator panel — rate, set difficulty, delete
- 🔒 Admin list loaded from database — no hardcoded names
- ⚡ Cancel tokens — no crashes when closing screens mid-request
- 💾 Rate info cache — instant repeated lookups

---

## 👥 Credits

| Role | Who |
|------|-----|
| 🛠️ Developer | **nekroz** |
| 🗄️ Backend | Turso LibSQL database |
| 🌍 Community | Everyone who submits and rates levels |

---

## ⚠️ Disclaimer

> Custom Rates is a **community project** and is not affiliated with
> RobTop Games in any way. All ratings are issued by community
> moderators and do not represent official Geometry Dash ratings.
> Level data (names, like counts) is fetched from GD servers
> purely for display purposes.

---

*Built with ❤️ for the Geometry Dash community*
