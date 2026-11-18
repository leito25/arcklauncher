import { expect, test } from "@playwright/test";

test("games, local stats and offline navigation", async ({ page, context }) => {
  await page.goto("/");
  await expect(page.getByRole("heading", { name: "Choose your game" })).toBeVisible();
  await page.getByRole("link", { name: "Play Snake" }).click();
  await expect(page.getByLabel("Snake game board")).toBeVisible();
  await page.getByRole("button", { name: "Move up" }).click();
  await expect(page.getByRole("status")).toContainText("Game over", { timeout: 10000 });
  await page.getByRole("link", { name: "Back to arcade" }).click();
  await expect(page.getByText("Best score: 0")).toBeVisible();
  await page.getByRole("link", { name: "Play WebGL Cube" }).click();
  await expect(page.getByLabel("Interactive WebGL cube")).toBeVisible();
  await expect(page.getByRole("alert")).toHaveCount(0);
  const cubeDrawn = await page.getByLabel("Interactive WebGL cube").evaluate(async (canvas: HTMLCanvasElement) => {
    const gl = canvas.getContext("webgl");
    if (!gl) return false;
    return await new Promise<boolean>((resolve) => requestAnimationFrame(() => {
      const pixels = new Uint8Array(canvas.width * canvas.height * 4);
      gl.readPixels(0, 0, canvas.width, canvas.height, gl.RGBA, gl.UNSIGNED_BYTE, pixels);
      resolve(pixels.some((value, index) => index % 4 === 2 && value > 100));
    }));
  });
  expect(cubeDrawn).toBe(true);
  await page.waitForTimeout(2100);
  await page.getByRole("button", { name: "Pause rotation" }).click();
  await expect(page.getByRole("button", { name: "Resume rotation" })).toBeVisible();
  await page.getByRole("link", { name: "Back to arcade" }).click();
  await expect.poll(async () => page.evaluate(async () => {
    const database = await new Promise<IDBDatabase>((resolve, reject) => {
      const request = indexedDB.open("arcade-portfolio", 1);
      request.onsuccess = () => resolve(request.result);
      request.onerror = () => reject(request.error);
    });
    const value = await new Promise<number>((resolve, reject) => {
      const request = database.transaction("profiles").objectStore("profiles").get("profile");
      request.onsuccess = () => resolve(request.result?.games?.["webgl-cube"]?.playtimeSeconds ?? 0);
      request.onerror = () => reject(request.error);
    });
    database.close();
    return value;
  })).toBeGreaterThanOrEqual(2);
  await page.reload();
  await expect(page.getByText("Best score: 0")).toBeVisible();
  await page.evaluate(async () => { await navigator.serviceWorker.ready; });
  await page.reload();
  await context.setOffline(true);
  await page.reload();
  await expect(page.getByRole("heading", { name: "Choose your game" })).toBeVisible();
  await page.getByRole("link", { name: "Play Snake" }).click();
  await expect(page.getByLabel("Snake game board")).toBeVisible();
  await page.getByRole("link", { name: "Back to arcade" }).click();
  await page.getByRole("link", { name: "Play WebGL Cube" }).click();
  await expect(page.getByLabel("Interactive WebGL cube")).toBeVisible();
});

test("supports a mobile viewport and touch controls", async ({ page }) => {
  await page.setViewportSize({ width: 375, height: 812 });
  await page.goto("/");
  await page.getByRole("link", { name: "Play Snake" }).click();
  await expect(page.getByRole("button", { name: "Move up" })).toBeVisible();
  await page.getByRole("button", { name: "Move up" }).click();
  await expect(page.getByRole("status")).toContainText("Game over", { timeout: 10000 });
  await page.getByRole("button", { name: "Play again" }).click();
  await expect(page.getByRole("status")).toHaveCount(0);
});

test("caches both games before either is opened", async ({ page, context }) => {
  await page.goto("/");
  await page.evaluate(async () => { await navigator.serviceWorker.ready; });
  await page.reload();
  await context.setOffline(true);
  await page.reload();
  await page.getByRole("link", { name: "Play Snake" }).click();
  await expect(page.getByLabel("Snake game board")).toBeVisible();
  await page.getByRole("link", { name: "Back to arcade" }).click();
  await page.getByRole("link", { name: "Play WebGL Cube" }).click();
  await expect(page.getByLabel("Interactive WebGL cube")).toBeVisible();
});

test("reports WebGL unavailability", async ({ page }) => {
  await page.addInitScript(() => {
    const original = HTMLCanvasElement.prototype.getContext;
    HTMLCanvasElement.prototype.getContext = function (...args: Parameters<typeof original>) {
      if (args[0] === "webgl") return null;
      return original.apply(this, args);
    } as typeof original;
  });
  await page.goto("/#/cube");
  await expect(page.getByRole("alert")).toContainText("WebGL is unavailable");
});
