<script setup>
import { onMounted, ref } from 'vue'
import { useRouter } from 'vue-router'
import { getHistory, getSessions, sendMessage, sendMessageToNewSession } from '../services/api'

const router = useRouter()
const sessions = ref([])
const activeSessionId = ref('')
const draft = ref('')
const messages = ref([])
const modelType = ref('1')
const loading = ref(false)
const tempMode = ref(false)

function asMessage(role, content) {
  return { role, content, id: Date.now() + Math.random() }
}

async function loadSessionList() {
  const { data } = await getSessions()
  if (data?.success && Array.isArray(data.sessions)) {
    sessions.value = data.sessions.map((s) => ({
      sessionId: String(s.sessionId),
      name: s.name || `Session ${s.sessionId}`
    }))
  }
}

async function switchSession(id) {
  activeSessionId.value = String(id)
  tempMode.value = false
  messages.value = []
  const { data } = await getHistory(activeSessionId.value)
  if (data?.success && Array.isArray(data.history)) {
    messages.value = data.history.map((item) =>
      asMessage(item.is_user ? 'user' : 'assistant', item.content)
    )
  }
}

function createTempSession() {
  tempMode.value = true
  activeSessionId.value = ''
  messages.value = []
}

async function submit() {
  const question = draft.value.trim()
  if (!question || loading.value) return

  loading.value = true
  messages.value.push(asMessage('user', question))

  try {
    if (tempMode.value) {
      const { data } = await sendMessageToNewSession({ question, modelType: modelType.value })
      if (data?.success) {
        const sid = String(data.sessionId)
        activeSessionId.value = sid
        tempMode.value = false
        messages.value.push(asMessage('assistant', data.Information || ''))
        await loadSessionList()
      } else {
        messages.value.push(asMessage('assistant', data?.message || 'Failed to create session'))
      }
    } else if (activeSessionId.value) {
      const { data } = await sendMessage({
        question,
        modelType: modelType.value,
        sessionId: activeSessionId.value
      })
      messages.value.push(asMessage('assistant', data?.success ? data.Information : (data?.message || 'Send failed')))
    } else {
      messages.value.push(asMessage('assistant', 'Please select or create a session first'))
    }
  } catch {
    messages.value.push(asMessage('assistant', 'Request failed, please retry later'))
  } finally {
    draft.value = ''
    loading.value = false
  }
}

onMounted(async () => {
  await loadSessionList()
})
</script>

<template>
  <main class="page-shell chat-wrap">
    <section class="card chat-card">
      <aside class="left-panel">
        <div class="head">
          <strong>Sessions</strong>
          <button class="button" @click="createTempSession">New Session</button>
        </div>
        <div class="session-list">
          <button
            v-for="item in sessions"
            :key="item.sessionId"
            class="session-item"
            :class="{ active: activeSessionId === item.sessionId && !tempMode }"
            @click="switchSession(item.sessionId)"
          >
            {{ item.name }}
          </button>
        </div>
      </aside>

      <section class="right-panel">
        <header>
          <button class="button secondary" @click="router.push('/menu')">Back</button>
          <select v-model="modelType" class="select model-select">
            <option value="1">General QA</option>
            <option value="2">Web Search</option>
            <option value="3">Knowledge RAG</option>
            <option value="4">Tool MCP</option>
          </select>
        </header>

        <div class="message-list">
          <div v-for="m in messages" :key="m.id" class="bubble" :class="m.role">
            <strong>{{ m.role === 'user' ? 'Me' : 'AI' }}</strong>
            <p>{{ m.content }}</p>
          </div>
        </div>

        <form class="composer" @submit.prevent="submit">
          <textarea v-model="draft" class="textarea" rows="3" placeholder="Enter your question..." />
          <button class="button" :disabled="loading" type="submit">{{ loading ? 'Sending...' : 'Send' }}</button>
        </form>
      </section>
    </section>
  </main>
</template>

<style scoped>
.chat-card {
  width: min(1240px, 96vw);
  min-height: 86vh;
  display: grid;
  grid-template-columns: 280px 1fr;
  overflow: hidden;
}

.left-panel {
  border-right: 1px solid var(--border);
  padding: 12px;
  background: var(--panel-strong);
}

.head {
  display: flex;
  justify-content: space-between;
  align-items: center;
  gap: 8px;
}

.session-list {
  margin-top: 12px;
  display: grid;
  gap: 8px;
  max-height: calc(86vh - 80px);
  overflow: auto;
}

.session-item {
  border: 1px solid var(--border);
  background: rgba(255, 255, 255, 0.04);
  color: var(--text);
  border-radius: 10px;
  padding: 10px;
  text-align: left;
  cursor: pointer;
}

.session-item.active {
  border-color: rgba(125, 162, 255, 0.8);
  background: rgba(125, 162, 255, 0.18);
}

.right-panel {
  display: grid;
  grid-template-rows: auto 1fr auto;
  gap: 10px;
  padding: 12px;
}

header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.model-select {
  max-width: 180px;
}

.message-list {
  border: 1px solid var(--border);
  border-radius: 12px;
  padding: 12px;
  overflow: auto;
  background: rgba(0, 0, 0, 0.12);
}

.bubble {
  max-width: 72%;
  margin-bottom: 10px;
  padding: 10px 12px;
  border-radius: 12px;
}

.bubble.user {
  margin-left: auto;
  background: rgba(125, 162, 255, 0.24);
}

.bubble.assistant {
  background: rgba(255, 255, 255, 0.08);
}

.bubble p {
  margin: 8px 0 0;
  white-space: pre-wrap;
}

.composer {
  display: grid;
  grid-template-columns: 1fr auto;
  gap: 10px;
}

@media (max-width: 900px) {
  .chat-card {
    grid-template-columns: 1fr;
  }

  .left-panel {
    border-right: none;
    border-bottom: 1px solid var(--border);
  }
}

@media (max-width: 640px) {
  .composer {
    grid-template-columns: 1fr;
  }
}
</style>