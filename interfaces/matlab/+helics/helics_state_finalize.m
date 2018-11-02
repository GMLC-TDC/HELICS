function v = helics_state_finalize()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1593856878);
  end
  v = vInitialized;
end
