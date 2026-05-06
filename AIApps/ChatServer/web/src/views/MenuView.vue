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
          <span class="menu-icon" aria-hidden="true" />
          <span class="menu-body">
            <span class="menu-name">AI 对话</span>
            <span class="menu-desc">多会话 · 多模式 · 历史记录</span>
          </span>
          <span class="menu-chevron" aria-hidden="true" />
        </button>
        <button type="button" class="menu-tile" @click="router.push('/upload')">
          <span class="menu-icon alt" aria-hidden="true" />
          <span class="menu-body">
            <span class="menu-name">图像识别</span>
            <span class="menu-desc">上传图片 · 分类与置信度</span>
          </span>
          <span class="menu-chevron" aria-hidden="true" />
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
  background: radial-gradient(ellipse 80% 60% at 50% 0%, rgba(255, 255, 255, 0.04), transparent 55%), #0a0a0a;
  color: #fafafa;
}

.menu-panel {
  width: min(520px, 100%);
  padding: 36px 30px 28px;
  border-radius: 24px;
  border: 1px solid rgba(255, 255, 255, 0.12);
  background: rgba(16, 16, 16, 0.92);
  box-shadow:
    0 28px 56px rgba(0, 0, 0, 0.48),
    inset 0 1px 0 rgba(255, 255, 255, 0.05);
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
  letter-spacing: 0.06em;
  color: #9ca3af;
}

.menu-grid {
  display: grid;
  grid-template-columns: 1fr;
  gap: 12px;
}

.menu-tile {
  display: flex;
  align-items: flex-start;
  gap: 14px;
  padding: 18px 16px;
  text-align: left;
  border-radius: 18px;
  border: 1px solid rgba(255, 255, 255, 0.1);
  background: rgba(0, 0, 0, 0.32);
  color: inherit;
  cursor: pointer;
  box-shadow: inset 0 1px 0 rgba(255, 255, 255, 0.04);
  transition: border-color 0.18s ease, background 0.18s ease, transform 0.18s ease, box-shadow 0.18s ease;
}

.menu-tile:hover {
  border-color: rgba(255, 255, 255, 0.2);
  background: rgba(255, 255, 255, 0.05);
  transform: translateY(-2px);
  box-shadow:
    0 12px 28px rgba(0, 0, 0, 0.35),
    inset 0 1px 0 rgba(255, 255, 255, 0.06);
}

.menu-tile:focus-visible {
  outline: 2px solid rgba(255, 255, 255, 0.45);
  outline-offset: 2px;
}

.menu-icon {
  width: 44px;
  height: 44px;
  flex-shrink: 0;
  border-radius: 14px;
  border: 1px solid rgba(255, 255, 255, 0.12);
  background: rgba(255, 255, 255, 0.06);
}

.menu-icon.alt {
  border-radius: 14px;
  background: repeating-linear-gradient(
    -40deg,
    rgba(255, 255, 255, 0.05),
    rgba(255, 255, 255, 0.05) 3px,
    transparent 3px,
    transparent 7px
  );
}

.menu-body {
  flex: 1;
  min-width: 0;
  display: grid;
  gap: 5px;
}

.menu-name {
  font-size: 0.9375rem;
  font-weight: 600;
  letter-spacing: 0.06em;
}

.menu-desc {
  font-size: 0.8125rem;
  line-height: 1.45;
  color: #9ca3af;
}

.menu-chevron {
  align-self: center;
  width: 7px;
  height: 7px;
  border-right: 1.5px solid #9ca3af;
  border-bottom: 1.5px solid #9ca3af;
  transform: rotate(-45deg);
  flex-shrink: 0;
  opacity: 0.75;
}

.menu-footer {
  margin-top: 28px;
  padding-top: 22px;
  border-top: 1px solid rgba(255, 255, 255, 0.08);
}

.menu-logout {
  width: 100%;
  padding: 13px 16px;
  font-size: 0.6875rem;
  font-weight: 600;
  letter-spacing: 0.16em;
  text-transform: uppercase;
  color: #9ca3af;
  background: rgba(255, 255, 255, 0.04);
  border: 1px solid rgba(255, 255, 255, 0.12);
  border-radius: 14px;
  cursor: pointer;
  transition: color 0.16s ease, border-color 0.16s ease, background 0.16s ease;
}

.menu-logout:hover {
  color: #e5e7eb;
  border-color: rgba(255, 255, 255, 0.22);
  background: rgba(255, 255, 255, 0.07);
}

@media (min-width: 560px) {
  .menu-grid {
    grid-template-columns: 1fr 1fr;
  }

  .menu-tile {
    flex-direction: column;
    min-height: 148px;
  }

  .menu-chevron {
    align-self: flex-end;
    margin-top: auto;
  }
}
</style>
