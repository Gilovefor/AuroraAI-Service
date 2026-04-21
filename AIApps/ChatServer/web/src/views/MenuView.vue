<script setup>
import { useRouter } from 'vue-router'
import { logout } from '../services/api'

const router = useRouter()

async function doLogout() {
  const userId = sessionStorage.getItem('userId')
  try {
    await logout({ userId, type: 'manual', gameType: 0 })
  } finally {
    sessionStorage.clear()
    router.push('/')
  }
}
</script>

<template>
  <main class="menu-shell">
    <section class="menu-panel">
      <header class="menu-header">
        <p class="menu-eyebrow">控制台</p>
        <h1 class="menu-title">选择功能</h1>
        <p class="menu-lead">对话与图像识别</p>
      </header>

      <div class="menu-grid">
        <button type="button" class="menu-tile" @click="router.push('/chat')">
          <span class="menu-tile-mark" aria-hidden="true" />
          <span class="menu-tile-body">
            <span class="menu-tile-name">AI 对话</span>
            <span class="menu-tile-desc">多会话 · 多模式 · 历史记录</span>
          </span>
          <span class="menu-tile-arrow" aria-hidden="true" />
        </button>
        <button type="button" class="menu-tile" @click="router.push('/upload')">
          <span class="menu-tile-mark alt" aria-hidden="true" />
          <span class="menu-tile-body">
            <span class="menu-tile-name">图像识别</span>
            <span class="menu-tile-desc">上传图片 · 分类与置信度</span>
          </span>
          <span class="menu-tile-arrow" aria-hidden="true" />
        </button>
      </div>

      <footer class="menu-footer">
        <button type="button" class="menu-logout" @click="doLogout">退出登录</button>
      </footer>
    </section>
  </main>
</template>

<style scoped>
.menu-shell {
  min-height: 100vh;
  min-height: 100dvh;
  margin: 0;
  padding: clamp(20px, 5vw, 40px);
  display: flex;
  align-items: center;
  justify-content: center;
  box-sizing: border-box;
  background: #0a0a0a;
  color: #fafafa;
}

.menu-panel {
  width: min(520px, 100%);
  padding: 36px 32px 28px;
  border: 1px solid rgba(255, 255, 255, 0.12);
  background: rgba(12, 12, 12, 0.92);
  box-shadow: 0 24px 48px rgba(0, 0, 0, 0.5);
}

.menu-header {
  text-align: center;
  margin-bottom: 28px;
}

.menu-eyebrow {
  margin: 0;
  font-size: 0.6875rem;
  font-weight: 600;
  letter-spacing: 0.16em;
  text-transform: uppercase;
  color: #6b7280;
}

.menu-title {
  margin: 10px 0 0;
  font-size: 1.375rem;
  font-weight: 600;
  letter-spacing: 0.14em;
  text-transform: uppercase;
}

.menu-lead {
  margin: 10px 0 0;
  font-size: 0.8125rem;
  letter-spacing: 0.08em;
  color: #9ca3af;
}

.menu-grid {
  display: grid;
  grid-template-columns: 1fr;
  gap: 10px;
}

.menu-tile {
  display: flex;
  align-items: flex-start;
  gap: 14px;
  padding: 18px 16px;
  text-align: left;
  border: 1px solid rgba(255, 255, 255, 0.1);
  background: rgba(0, 0, 0, 0.35);
  color: inherit;
  cursor: pointer;
  transition: border-color 0.15s ease, background 0.15s ease;
}

.menu-tile:hover {
  border-color: rgba(255, 255, 255, 0.22);
  background: rgba(255, 255, 255, 0.04);
}

.menu-tile:focus-visible {
  outline: 2px solid #e5e5e5;
  outline-offset: 2px;
}

.menu-tile-mark {
  width: 40px;
  height: 40px;
  flex-shrink: 0;
  border: 1px solid rgba(255, 255, 255, 0.15);
  background: rgba(255, 255, 255, 0.06);
}

.menu-tile-mark.alt {
  border-radius: 0;
  background: repeating-linear-gradient(
    -45deg,
    rgba(255, 255, 255, 0.04),
    rgba(255, 255, 255, 0.04) 4px,
    transparent 4px,
    transparent 8px
  );
}

.menu-tile-body {
  flex: 1;
  min-width: 0;
  display: grid;
  gap: 4px;
}

.menu-tile-name {
  font-size: 0.9375rem;
  font-weight: 600;
  letter-spacing: 0.06em;
}

.menu-tile-desc {
  font-size: 0.8125rem;
  line-height: 1.45;
  color: #9ca3af;
}

.menu-tile-arrow {
  align-self: center;
  width: 6px;
  height: 6px;
  border-right: 1px solid #9ca3af;
  border-bottom: 1px solid #9ca3af;
  transform: rotate(-45deg);
  flex-shrink: 0;
  opacity: 0.8;
}

.menu-footer {
  margin-top: 28px;
  padding-top: 22px;
  border-top: 1px solid rgba(255, 255, 255, 0.1);
}

.menu-logout {
  width: 100%;
  padding: 12px 16px;
  font-size: 0.6875rem;
  font-weight: 600;
  letter-spacing: 0.18em;
  text-transform: uppercase;
  color: #9ca3af;
  background: transparent;
  border: 1px solid rgba(255, 255, 255, 0.14);
  cursor: pointer;
  transition: color 0.15s ease, border-color 0.15s ease, background 0.15s ease;
}

.menu-logout:hover {
  color: #e5e7eb;
  border-color: rgba(255, 255, 255, 0.28);
  background: rgba(255, 255, 255, 0.04);
}

@media (min-width: 560px) {
  .menu-grid {
    grid-template-columns: 1fr 1fr;
  }

  .menu-tile {
    flex-direction: column;
    min-height: 140px;
  }

  .menu-tile-arrow {
    align-self: flex-end;
    margin-top: auto;
  }
}
</style>
