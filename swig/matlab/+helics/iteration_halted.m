function v = iteration_halted()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1432107602);
  end
  v = vInitialized;
end
