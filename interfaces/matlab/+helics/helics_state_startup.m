function v = helics_state_startup()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1398230872);
  end
  v = vInitialized;
end
