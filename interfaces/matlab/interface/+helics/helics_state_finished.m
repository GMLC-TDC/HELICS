function v = helics_state_finished()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 133);
  end
  v = vInitialized;
end
