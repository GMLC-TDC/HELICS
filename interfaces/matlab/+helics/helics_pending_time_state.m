function v = helics_pending_time_state()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1398230879);
  end
  v = vInitialized;
end
